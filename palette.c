#include "include/turtleTools.h"
#include "include/osTools.h"
#include <time.h>

#define NUMBER_OF_BOXES 23

void export(const char *filename);
void import(const char *filename);

/* box */
typedef struct {
    double red;
    double green;
    double blue;
    double redSave;
    double greenSave;
    double blueSave;
    double x;
    double y;
    double size;
    int32_t status;
    int32_t saveStatus;
    tt_slider_t *sliders[3];
} box_t;

/* global state */
typedef struct {
    uint32_t width;
    double boxSize;
    double topX;
    double topY;
    box_t boxes[NUMBER_OF_BOXES];
    double newColorPalette[NUMBER_OF_BOXES * 6];
    char lockOffsets;
    char asciiEnum[NUMBER_OF_BOXES][32];
    double boxSliderColors[9];
    double boxSliderColorsAlt[9];
    list_t *undoList;
    uint32_t undoIndex;
    uint32_t firstUndo;
    uint32_t firstRedo;
    int8_t keys[10];
    uint32_t copyMessage;
    int32_t dragBox;
    int32_t saved;
} palette_t;

palette_t self;
tt_popup_t fakePopup;

/* UI variables */
int32_t buttonVar, switchVar = 0, dropdownVar = 0;
double dialVar = 0.0, sliderVar = 0.0;

void init() {
    /* setup UI */
    list_t *dropdownOptions = list_init();
    list_append(dropdownOptions, (unitype) "dropA", 's');
    list_append(dropdownOptions, (unitype) "dropB", 's');
    list_append(dropdownOptions, (unitype) "dropC", 's');
    list_append(dropdownOptions, (unitype) "dropD", 's');
    list_append(dropdownOptions, (unitype) "dropE", 's');
    double UIX = 200;
    double UIY = 100;
    buttonInit("button", &buttonVar, TT_BUTTON_SHAPE_RECTANGLE, UIX, UIY, 10);
    switchInit("switch", &switchVar, UIX, UIY - 40, 10);
    dialInit("dial", &dialVar, TT_DIAL_EXP, UIX, UIY - 80, 10, 0, 1000, 1);
    sliderInit("slider", &sliderVar, TT_SLIDER_HORIZONTAL, TT_SLIDER_ALIGN_CENTER, UIX, UIY - 120, 10, 50, 0, 10, 1);
    sliderInit("slider", &sliderVar, TT_SLIDER_VERTICAL, TT_SLIDER_ALIGN_CENTER, UIX - 50, UIY - 120, 10, 50, 0, 10, 1);
    dropdownInit("dropdown", dropdownOptions, &dropdownVar, TT_DROPDOWN_ALIGN_CENTER, UIX, UIY - 165, 10);
    double boxSliderCopy[] = {
        0.0, 0.0, 0.0,       // override slider text
        0.0, 0.0, 0.0,       // override slider bar
        255.0, 255.0, 255.0, // override slider circle
    };
    memcpy(self.boxSliderColors, boxSliderCopy, sizeof(boxSliderCopy));
    double boxSliderAltCopy[] = {
        0.0, 0.0, 0.0,       // override slider text
        255.0, 255.0, 255.0, // override slider bar
        0.0, 0.0, 0.0,       // override slider circle
    };
    memcpy(self.boxSliderColorsAlt, boxSliderAltCopy, sizeof(boxSliderAltCopy));
    /* init fakePopup */
    fakePopup.minX = UIX - 150;
    fakePopup.minY = UIY - 31;
    fakePopup.maxX = UIX - 30;
    fakePopup.maxY = UIY + 9;
    fakePopup.output[0] = 0;
    fakePopup.output[1] = -1;
    fakePopup.mouseDown = 0;
    fakePopup.style = 0;
    fakePopup.message = strdup("Are you sure you want to close?");
    fakePopup.options = list_init();
    list_append(fakePopup.options, (unitype) strdup("Yes"), 's');
    list_append(fakePopup.options, (unitype) strdup("No"), 's');

    /* setup asciiEnum */
    for (uint32_t i = 0; i < NUMBER_OF_BOXES; i++) {
        strcpy(self.asciiEnum[i], "NULL");
    }
    char asciiCopy[][32] = {
        "BACKGROUND",
        "TT_COLOR_TEXT",
        "TT_COLOR_TEXT_ALTERNATE",
        "TT_COLOR_RIBBON_TOP",
        "TT_COLOR_RIBBON_DROPDOWN",
        "TT_COLOR_RIBBON_SELECT",
        "TT_COLOR_POPUP_BOX",
        "TT_COLOR_POPUP_BUTTON",
        "TT_COLOR_POPUP_BUTTON_SELECT",
        "TT_COLOR_BUTTON",
        "TT_COLOR_BUTTON_SELECT",
        "TT_COLOR_SWITCH_OFF",
        "TT_COLOR_SWITCH_CIRCLE_OFF",
        "TT_COLOR_SWITCH_ON",
        "TT_COLOR_SWITCH_CIRCLE_ON",
        "TT_COLOR_DIAL",
        "TT_COLOR_DIAL_INNER",
        "TT_COLOR_SLIDER_BAR",
        "TT_COLOR_SLIDER_CIRCLE",
        "TT_COLOR_DROPDOWN",
        "TT_COLOR_DROPDOWN_SELECT",
        "TT_COLOR_DROPDOWN_HOVER",
        "TT_COLOR_DROPDOWN_TRIANGLE",
    };
    for (uint32_t i = 0; i < sizeof(asciiCopy) / 32; i++) {
        strcpy(self.asciiEnum[i], asciiCopy[i]);
    }

    /* boxes */
    self.lockOffsets = 0;
    self.width = 5;
    self.boxSize = 60;
    self.topX = -310 + self.boxSize * 0.5;
    self.topY = 160 - self.boxSize * 0.5;
    for (uint32_t i = 0; i < NUMBER_OF_BOXES; i++) {
        self.boxes[i].size = self.boxSize;
        self.boxes[i].status = 0;
        self.boxes[i].saveStatus = 0;
        self.boxes[i].red = randomDouble(0, 255);
        self.boxes[i].green = randomDouble(0, 255);
        self.boxes[i].blue = randomDouble(0, 255);
        self.newColorPalette[i * 6 + 0] = self.boxes[i].red;
        self.newColorPalette[i * 6 + 1] = self.boxes[i].redSave;
        self.newColorPalette[i * 6 + 2] = self.boxes[i].green;
        self.newColorPalette[i * 6 + 3] = self.boxes[i].greenSave;
        self.newColorPalette[i * 6 + 4] = self.boxes[i].blue;
        self.newColorPalette[i * 6 + 5] = self.boxes[i].blueSave;
        self.boxes[i].x = self.topX + self.boxSize * (i % self.width) * 1.05;
        self.boxes[i].y = self.topY - self.boxSize * (i / self.width) * 1.05;
        self.boxes[i].sliders[0] = sliderInit("", &(self.boxes[i].red), TT_SLIDER_HORIZONTAL, TT_SLIDER_ALIGN_CENTER, self.boxes[i].x, self.boxes[i].y + 20, 5, self.boxSize * 0.8, 0, 255, 0);
        self.boxes[i].sliders[1] = sliderInit("", &(self.boxes[i].green), TT_SLIDER_HORIZONTAL, TT_SLIDER_ALIGN_CENTER, self.boxes[i].x, self.boxes[i].y + 10, 5, self.boxSize * 0.8, 0, 255, 0);
        self.boxes[i].sliders[2] = sliderInit("", &(self.boxes[i].blue), TT_SLIDER_HORIZONTAL, TT_SLIDER_ALIGN_CENTER, self.boxes[i].x, self.boxes[i].y, 5, self.boxSize * 0.8, 0, 255, 0);
        tt_colorOverride((void *) self.boxes[i].sliders[0], self.boxSliderColors, 9);
        tt_colorOverride((void *) self.boxes[i].sliders[1], self.boxSliderColors, 9);
        tt_colorOverride((void *) self.boxes[i].sliders[2], self.boxSliderColors, 9);
    }

    self.undoList = list_init();
    self.undoIndex = 0;
    self.copyMessage = 0;
    self.dragBox = -1;
    self.saved = 1;
    self.firstUndo = 1;
    self.firstRedo = 1;
    memset(self.keys, 0, sizeof(self.keys));
}

void addToUndo() {
    while (self.undoList -> length > self.undoIndex) {
        list_pop(self.undoList);
    }
    // if (self.firstUndo == 1) {
        for (uint32_t i = 0; i < NUMBER_OF_BOXES; i++) {
            list_append(self.undoList, (unitype) self.newColorPalette[i * 6 + 0], 'd');
            list_append(self.undoList, (unitype) self.newColorPalette[i * 6 + 1], 'd');
            list_append(self.undoList, (unitype) self.newColorPalette[i * 6 + 2], 'd');
            list_append(self.undoList, (unitype) self.newColorPalette[i * 6 + 3], 'd');
            list_append(self.undoList, (unitype) self.newColorPalette[i * 6 + 4], 'd');
            list_append(self.undoList, (unitype) self.newColorPalette[i * 6 + 5], 'd');
        }
    // }
    self.firstUndo = 1;
    self.firstRedo = 1;
    self.undoIndex = self.undoList -> length;
    printf("add to undo %d\n", self.undoIndex);
    self.saved = 0;
}

void undo() {
    printf("undo %d\n", self.undoIndex);
    if (self.firstUndo == 1) {
        for (uint32_t i = 0; i < NUMBER_OF_BOXES; i++) {
            list_append(self.undoList, (unitype) self.boxes[i].red, 'd');
            list_append(self.undoList, (unitype) self.boxes[i].redSave, 'd');
            list_append(self.undoList, (unitype) self.boxes[i].green, 'd');
            list_append(self.undoList, (unitype) self.boxes[i].greenSave, 'd');
            list_append(self.undoList, (unitype) self.boxes[i].blue, 'd');
            list_append(self.undoList, (unitype) self.boxes[i].blueSave, 'd');
        }
        self.firstUndo = 0;
    }
    if (!self.firstRedo) {
        self.undoIndex -= 6 * NUMBER_OF_BOXES;
    }
    if (self.undoIndex >= 6 * NUMBER_OF_BOXES) {
        self.firstRedo = 1;
        for (int32_t i = NUMBER_OF_BOXES - 1; i >= 0; i--) {
            self.undoIndex--;
            self.boxes[i].blueSave = self.undoList -> data[self.undoIndex].d;
            self.undoIndex--;
            self.boxes[i].blue = self.undoList -> data[self.undoIndex].d;
            self.undoIndex--;
            self.boxes[i].greenSave = self.undoList -> data[self.undoIndex].d;
            self.undoIndex--;
            self.boxes[i].green = self.undoList -> data[self.undoIndex].d;
            self.undoIndex--;
            self.boxes[i].redSave = self.undoList -> data[self.undoIndex].d;
            self.undoIndex--;
            self.boxes[i].red = self.undoList -> data[self.undoIndex].d;
        }
        self.saved = -2;
    }
}

void redo() {
    printf("redo %d\n", self.undoIndex);
    if (self.undoIndex < self.undoList -> length) {
        if (self.firstRedo == 1) {
            self.undoIndex += 6 * NUMBER_OF_BOXES;
            self.firstRedo = 0;
        }
        self.undoIndex += 6 * NUMBER_OF_BOXES;
        for (int32_t i = NUMBER_OF_BOXES - 1; i >= 0; i--) {
            self.undoIndex--;
            self.boxes[i].blueSave = self.undoList -> data[self.undoIndex].d;
            self.undoIndex--;
            self.boxes[i].blue = self.undoList -> data[self.undoIndex].d;
            self.undoIndex--;
            self.boxes[i].greenSave = self.undoList -> data[self.undoIndex].d;
            self.undoIndex--;
            self.boxes[i].green = self.undoList -> data[self.undoIndex].d;
            self.undoIndex--;
            self.boxes[i].redSave = self.undoList -> data[self.undoIndex].d;
            self.undoIndex--;
            self.boxes[i].red = self.undoList -> data[self.undoIndex].d;
        }
        self.undoIndex += 6 * NUMBER_OF_BOXES;
        self.saved = -2;
    }
}

void renderBoxes() {
    /* render background */
    turtlePenColor(self.boxes[0].red, self.boxes[0].green, self.boxes[0].blue);
    turtleRectangle(self.topX + self.boxSize * self.width, -180, 320, 180);
    /* render boxes */
    for (uint32_t i = 0; i < NUMBER_OF_BOXES; i++) {
        turtlePenColor(self.boxes[i].red, self.boxes[i].green, self.boxes[i].blue);
        turtleRectangle(self.boxes[i].x - self.boxes[i].size * 0.5, self.boxes[i].y - self.boxes[i].size * 0.5, self.boxes[i].x + self.boxes[i].size * 0.5, self.boxes[i].y + self.boxes[i].size * 0.5);
        char rgbStr[48];
        char hexStr[12];
        sprintf(rgbStr, "%d, %d, %d", (int) round(self.boxes[i].red), (int) round(self.boxes[i].green), (int) round(self.boxes[i].blue));
        sprintf(hexStr, "#%02X%02X%02X", (int) round(self.boxes[i].red), (int) round(self.boxes[i].green), (int) round(self.boxes[i].blue));
        if (self.boxes[i].red + self.boxes[i].green + self.boxes[i].blue < 150) {
            turtlePenColor(255, 255, 255);
            tt_colorOverride((void *) self.boxes[i].sliders[0], self.boxSliderColorsAlt, 9);
            tt_colorOverride((void *) self.boxes[i].sliders[1], self.boxSliderColorsAlt, 9);
            tt_colorOverride((void *) self.boxes[i].sliders[2], self.boxSliderColorsAlt, 9);
        } else {
            turtlePenColor(0, 0, 0);
            tt_colorOverride((void *) self.boxes[i].sliders[0], self.boxSliderColors, 9);
            tt_colorOverride((void *) self.boxes[i].sliders[1], self.boxSliderColors, 9);
            tt_colorOverride((void *) self.boxes[i].sliders[2], self.boxSliderColors, 9);
        }
        turtleTextWriteString(rgbStr, self.boxes[i].x, self.boxes[i].y - 10, self.boxes[i].size / 10, 50);
        turtleTextWriteString(hexStr, self.boxes[i].x, self.boxes[i].y - 20, self.boxes[i].size / 10, 50);
    }
    /* set color palette */
    double setColorPalette[NUMBER_OF_BOXES * 3]; // this is so terrible
    for (uint32_t i = 0; i < NUMBER_OF_BOXES; i++) {
        if (!turtleMouseDown() && (fabs(self.newColorPalette[i * 6 + 0] - self.boxes[i].red) > 0.5 || fabs(self.newColorPalette[i * 6 + 2] - self.boxes[i].green) > 0.5 || fabs(self.newColorPalette[i * 6 + 4] - self.boxes[i].blue)) > 0.5) {
            if (self.saved >= 0) {
                addToUndo();
            }
            self.newColorPalette[i * 6 + 0] = self.boxes[i].red;
            self.newColorPalette[i * 6 + 1] = self.boxes[i].redSave;
            self.newColorPalette[i * 6 + 2] = self.boxes[i].green;
            self.newColorPalette[i * 6 + 3] = self.boxes[i].greenSave;
            self.newColorPalette[i * 6 + 4] = self.boxes[i].blue;
            self.newColorPalette[i * 6 + 5] = self.boxes[i].blueSave;
        }
        setColorPalette[i * 3 + 0] = self.boxes[i].red;
        setColorPalette[i * 3 + 1] = self.boxes[i].green;
        setColorPalette[i * 3 + 2] = self.boxes[i].blue;
    }
    if (self.saved == -1) {
        self.saved = 1;
    }
    if (self.saved == -2) {
        self.saved = 0;
    }
    memcpy(tt_themeColors, setColorPalette + 3, sizeof(tt_themeColors));
    /* display copy message */
    if (self.copyMessage > 0) {
        self.copyMessage--;
        char copyText[64] = "copied ";
        osToolsClipboardGetText();
        memcpy(copyText + 7, osToolsClipboard.text, strlen(osToolsClipboard.text));
        turtlePenColorAlpha(self.boxes[1].red, self.boxes[1].green, self.boxes[1].blue, 255 - self.copyMessage);
        turtleTextWriteString(copyText, self.topX + self.boxSize * self.width + 5, 160, 8, 0);
    }
}

void displaySaveIndicator() {
    /* display saved indicator */
    if (self.saved == 0) {
        turtlePenColor(self.boxes[18].red, self.boxes[18].green, self.boxes[18].blue);
        turtlePenSize(5);
        turtleGoto(310, 175);
        turtlePenDown();
        turtlePenUp();
    }
}

void renderFakePopup() {
    tt_setColor(TT_COLOR_POPUP_BOX);
    turtleRectangle(fakePopup.minX, fakePopup.minY, fakePopup.maxX, fakePopup.maxY);
    double textSize = 5;
    double textX = fakePopup.minX + (fakePopup.maxX - fakePopup.minX) / 2;
    double textY = fakePopup.maxY - textSize * 2;
    tt_setColor(TT_COLOR_TEXT_ALTERNATE);
    turtleTextWriteUnicode((unsigned char *) fakePopup.message, textX, textY, textSize, 50);
    textY -= textSize * 4;
    double fullLength = 0;
    for (uint32_t i = 0; i < fakePopup.options -> length; i++) {
        fullLength += turtleTextGetStringLength(fakePopup.options -> data[i].s, textSize);
    }
    /* we have the length of the strings, now we pad with n + 1 padding regions */
    double padThai = (fakePopup.maxX - fakePopup.minX - fullLength) / (fakePopup.options -> length + 1);
    textX = fakePopup.minX + padThai;
    char flagged = 0;
    if (!turtleMouseDown() && fakePopup.mouseDown == 1) {
        flagged = 1; // flagged for mouse misbehaviour
    }
    for (uint32_t i = 0; i < fakePopup.options -> length; i++) {
        double strLen = turtleTextGetStringLength(fakePopup.options -> data[i].s, textSize);
        if (turtle.mouseX > textX - textSize && turtle.mouseX < textX + strLen + textSize &&
        turtle.mouseY > textY - textSize && turtle.mouseY < textY + textSize) {
            tt_setColor(TT_COLOR_POPUP_BUTTON_SELECT);
            turtleRectangle(textX - textSize, textY - textSize, textX + textSize + strLen, textY + textSize);
            if (turtleMouseDown()) {
                if (fakePopup.mouseDown == 0) {
                    fakePopup.mouseDown = 1;
                    if (fakePopup.output[0] == 0) {
                        fakePopup.output[1] = i;
                    }
                }
            } else {
                if (fakePopup.mouseDown == 1) {
                    fakePopup.mouseDown = 0;
                    if (fakePopup.output[1] == (int32_t) i) {
                        fakePopup.output[0] = 1;
                    }
                }
            }
        } else {
            tt_setColor(TT_COLOR_POPUP_BUTTON);
            turtleRectangle(textX - textSize, textY - textSize, textX + textSize + strLen, textY + textSize);
        }
        tt_setColor(TT_COLOR_TEXT_ALTERNATE);
        turtleTextWriteUnicode((unsigned char *) fakePopup.options -> data[i].s, textX, textY, textSize, 0);
        textX += strLen + padThai;
    }
    if (!turtleMouseDown() && fakePopup.mouseDown == 1 && flagged == 1) {
        fakePopup.mouseDown = 0;
        fakePopup.output[0] = 0;
        fakePopup.output[1] = -1;
    }
}

void mouseTick() {
    /* mouse */
    for (uint32_t i = 0; i < NUMBER_OF_BOXES; i++) {
        if (turtleMouseDown()) {
            if (self.boxes[i].status < 0) {
                self.boxes[i].status *= -1;
                char rgbStr[48];
                sprintf(rgbStr, "%d, %d, %d", (int) round(self.boxes[i].red), (int) round(self.boxes[i].green), (int) round(self.boxes[i].blue));
                osToolsClipboardSetText(rgbStr);
                self.dragBox = i;
                self.copyMessage = 255;
            }
        } else if (turtleMouseRight()) {
            if (self.boxes[i].status < 0) {
                self.boxes[i].status *= -1;
                char hexStr[12];
                sprintf(hexStr, "#%02X%02X%02X", (int) round(self.boxes[i].red), (int) round(self.boxes[i].green), (int) round(self.boxes[i].blue));
                osToolsClipboardSetText(hexStr);
                self.dragBox = i;
                self.copyMessage = 255;
            }
        } else {
            if (turtle.mouseX > self.boxes[i].x - self.boxes[i].size * 0.5 && turtle.mouseX < self.boxes[i].x + self.boxes[i].size * 0.5 &&
                turtle.mouseY > self.boxes[i].y - self.boxes[i].size * 0.5 && turtle.mouseY < self.boxes[i].y - self.boxes[i].size * 0.05) {
                self.boxes[i].status = -1;
            } else {
                self.boxes[i].status = 0;
            }
            if (self.dragBox != -1) {
                self.dragBox = -1;
                // addToUndo();
            }
        }
        if (self.boxes[i].status == 1) {
            turtlePenColor(self.boxes[i].red, self.boxes[i].green, self.boxes[i].blue);
            char rgbStr[48];
            char hexStr[12];
            sprintf(rgbStr, "%d, %d, %d", (int) round(self.boxes[i].red), (int) round(self.boxes[i].green), (int) round(self.boxes[i].blue));
            sprintf(hexStr, "#%02X%02X%02X", (int) round(self.boxes[i].red), (int) round(self.boxes[i].green), (int) round(self.boxes[i].blue));
            double boxWidth = turtleTextGetStringLength(rgbStr, self.boxes[i].size / 10);
            turtleRectangle(turtle.mouseX - boxWidth / 2 - 2, turtle.mouseY - 12, turtle.mouseX + boxWidth / 2 + 2, turtle.mouseY + 12);
            sprintf(hexStr, "#%02X%02X%02X", (int) round(self.boxes[i].red), (int) round(self.boxes[i].green), (int) round(self.boxes[i].blue));
            if (self.boxes[i].red + self.boxes[i].green + self.boxes[i].blue < 150) {
                turtlePenColor(255, 255, 255);
            } else {
                turtlePenColor(0, 0, 0);
            }
            turtleTextWriteString(rgbStr, turtle.mouseX, turtle.mouseY + 5, self.boxes[i].size / 10, 50);
            turtleTextWriteString(hexStr, turtle.mouseX, turtle.mouseY - 5, self.boxes[i].size / 10, 50);
        }
        if (self.dragBox >= 0) {
            if (turtle.mouseX > self.boxes[i].x - self.boxes[i].size * 0.5 && turtle.mouseX < self.boxes[i].x + self.boxes[i].size * 0.5 &&
                turtle.mouseY > self.boxes[i].y - self.boxes[i].size * 0.5 && turtle.mouseY < self.boxes[i].y + self.boxes[i].size * 0.5) {
                if (self.boxes[i].saveStatus == 0) {
                    self.boxes[i].saveStatus = 1;
                    self.boxes[i].redSave = self.boxes[i].red;
                    self.boxes[i].greenSave = self.boxes[i].green;
                    self.boxes[i].blueSave = self.boxes[i].blue;
                    self.boxes[i].red = self.boxes[self.dragBox].red;
                    self.boxes[i].green = self.boxes[self.dragBox].green;
                    self.boxes[i].blue = self.boxes[self.dragBox].blue;
                }
            } else {
                if (self.boxes[i].saveStatus == 1) {
                    self.boxes[i].red = self.boxes[i].redSave;
                    self.boxes[i].green = self.boxes[i].greenSave;
                    self.boxes[i].blue = self.boxes[i].blueSave;
                }
                self.boxes[i].saveStatus = 0;
            }
        } else {
            self.boxes[i].saveStatus = 0;
        }
    }
    for (uint32_t i = 0; i < NUMBER_OF_BOXES; i++) {
        if (self.boxes[i].red + self.boxes[i].green + self.boxes[i].blue < 150) {
            turtlePenColor(255, 255, 255);
        } else {
            turtlePenColor(0, 0, 0);
        }
        if (turtle.mouseX > self.boxes[i].x - self.boxes[i].size * 0.5 && turtle.mouseX < self.boxes[i].x + self.boxes[i].size * 0.5 &&
            turtle.mouseY > self.boxes[i].y - self.boxes[i].size * 0.5 && turtle.mouseY < self.boxes[i].y + self.boxes[i].size * 0.5) {
            turtlePenColor(255, 255, 255);
            turtleRectangle(turtle.mouseX, turtle.mouseY - 1, turtle.mouseX + turtleTextGetStringLength(self.asciiEnum[i], 5) + 4, turtle.mouseY + 10);
            turtlePenColor(0, 0, 0);
            turtleTextWriteString(self.asciiEnum[i], turtle.mouseX + 2, turtle.mouseY + 4, 5, 0);
        }
    }
    if (turtleKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
        if (self.lockOffsets == 0) {
            self.lockOffsets = 1;
            for (uint32_t i = 0; i < NUMBER_OF_BOXES; i++) {
                self.boxes[i].redSave = self.boxes[i].red;
                self.boxes[i].greenSave = self.boxes[i].green;
                self.boxes[i].blueSave = self.boxes[i].blue;
            }
        }
    } else {
        self.lockOffsets = 0;
    }
    if (turtleKeyPressed(GLFW_KEY_LEFT_CONTROL)) {
        if (self.keys[3] == 0) {
            self.keys[3] = 1;
        }
    } else {
        self.keys[3] = 0;
    }
    if (turtleKeyPressed(GLFW_KEY_S)) {
        if (self.keys[4] == 0) {
            self.keys[4] = 1;
            if (self.keys[3]) {
                if (strcmp(osToolsFileDialog.selectedFilename, "null") == 0) {
                    if (osToolsFileDialogPrompt(1, "") != -1) {
                        printf("Saved to: %s\n", osToolsFileDialog.selectedFilename);
                        export(osToolsFileDialog.selectedFilename);
                    }
                } else {
                    printf("Saved to: %s\n", osToolsFileDialog.selectedFilename);
                    export(osToolsFileDialog.selectedFilename);
                }
            }
        }
    } else {
        self.keys[4] = 0;
    }
    if (turtleKeyPressed(GLFW_KEY_Z)) {
        if (self.keys[5] == 0) {
            self.keys[5] = 1;
            if (self.keys[3]) {
                undo();
            }
        }
    } else {
        self.keys[5] = 0;
    }
    if (turtleKeyPressed(GLFW_KEY_Y)) {
        if (self.keys[6] == 0) {
            self.keys[6] = 1;
            if (self.keys[3]) {
                redo();
            }
        }
    } else {
        self.keys[6] = 0;
    }
    if (turtleKeyPressed(GLFW_KEY_V)) {
        if (self.keys[7] == 0) {
            self.keys[7] = 1;
            if (self.keys[3]) {
                for (uint32_t i = 0; i < NUMBER_OF_BOXES; i++) {
                    if (turtle.mouseX > self.boxes[i].x - self.boxes[i].size * 0.5 && turtle.mouseX < self.boxes[i].x + self.boxes[i].size * 0.5 &&
                        turtle.mouseY > self.boxes[i].y - self.boxes[i].size * 0.5 && turtle.mouseY < self.boxes[i].y + self.boxes[i].size * 0.5) {
                        osToolsClipboardGetText();
                        if (osToolsClipboard.text[0] == '#') {
                            /* assume #FFFFFF */
                            char sect[3] = {0};
                            int32_t hexOut;
                            sect[0] = osToolsClipboard.text[1];
                            sect[1] = osToolsClipboard.text[2];
                            sscanf(sect, "%X", &hexOut);
                            self.boxes[i].red = hexOut;
                            sect[0] = osToolsClipboard.text[3];
                            sect[1] = osToolsClipboard.text[4];
                            sscanf(sect, "%X", &hexOut);
                            self.boxes[i].green = hexOut;
                            sect[0] = osToolsClipboard.text[5];
                            sect[1] = osToolsClipboard.text[6];
                            sscanf(sect, "%X", &hexOut);
                            self.boxes[i].blue = hexOut;
                        } else if (strlen(osToolsClipboard.text) == 6) {
                            /* assume FFFFFF */
                            char sect[3] = {0};
                            int32_t hexOut;
                            sect[0] = osToolsClipboard.text[0];
                            sect[1] = osToolsClipboard.text[1];
                            sscanf(sect, "%X", &hexOut);
                            self.boxes[i].red = hexOut;
                            sect[0] = osToolsClipboard.text[2];
                            sect[1] = osToolsClipboard.text[3];
                            sscanf(sect, "%X", &hexOut);
                            self.boxes[i].green = hexOut;
                            sect[0] = osToolsClipboard.text[4];
                            sect[1] = osToolsClipboard.text[5];
                            sscanf(sect, "%X", &hexOut);
                            self.boxes[i].blue = hexOut;
                        } else {
                            /* assume R, G, B */
                            sscanf(osToolsClipboard.text, "%lf, %lf, %lf", &self.boxes[i].red, &self.boxes[i].green, &self.boxes[i].blue);
                        }
                    }
                }
            }
        }
    } else {
        self.keys[7] = 0;
    }
    if (self.lockOffsets) {
        for (uint32_t i = 0; i < NUMBER_OF_BOXES; i++) {
            /* this is really bad programming, but im not smart enough to figure out the right way to do it */
            if (self.boxes[i].red != self.boxes[i].redSave) {
                double offset = self.boxes[i].red - self.boxes[i].redSave;
                if (self.boxes[i].greenSave + offset > 255) {
                    offset = 255 - self.boxes[i].greenSave;
                }
                if (self.boxes[i].blueSave + offset > 255) {
                    offset = 255 - self.boxes[i].blueSave;
                }
                if (self.boxes[i].greenSave + offset < 0) {
                    offset = 0 - self.boxes[i].greenSave;
                }
                if (self.boxes[i].blueSave + offset < 0) {
                    offset = 0 - self.boxes[i].blueSave;
                }
                self.boxes[i].red = self.boxes[i].redSave + offset;
                self.boxes[i].green = self.boxes[i].greenSave + offset;
                self.boxes[i].blue = self.boxes[i].blueSave + offset;
                self.boxes[i].greenSave = self.boxes[i].green;
                self.boxes[i].blueSave = self.boxes[i].blue;
                self.boxes[i].redSave = self.boxes[i].red;
            }
            if (self.boxes[i].green != self.boxes[i].greenSave) {
                double offset = self.boxes[i].green - self.boxes[i].greenSave;
                if (self.boxes[i].redSave + offset > 255) {
                    offset = 255 - self.boxes[i].redSave;
                }
                if (self.boxes[i].blueSave + offset > 255) {
                    offset = 255 - self.boxes[i].blueSave;
                }
                if (self.boxes[i].redSave + offset < 0) {
                    offset = 0 - self.boxes[i].redSave;
                }
                if (self.boxes[i].blueSave + offset < 0) {
                    offset = 0 - self.boxes[i].blueSave;
                }
                self.boxes[i].green = self.boxes[i].greenSave + offset;
                self.boxes[i].red = self.boxes[i].redSave + offset;
                self.boxes[i].blue = self.boxes[i].blueSave + offset;
                self.boxes[i].redSave = self.boxes[i].red;
                self.boxes[i].blueSave = self.boxes[i].blue;
                self.boxes[i].greenSave = self.boxes[i].green;
            }
            if (self.boxes[i].blue != self.boxes[i].blueSave) {
                double offset = self.boxes[i].blue - self.boxes[i].blueSave;
                if (self.boxes[i].greenSave + offset > 255) {
                    offset = 255 - self.boxes[i].greenSave;
                }
                if (self.boxes[i].redSave + offset > 255) {
                    offset = 255 - self.boxes[i].redSave;
                }
                if (self.boxes[i].greenSave + offset < 0) {
                    offset = 0 - self.boxes[i].greenSave;
                }
                if (self.boxes[i].redSave + offset < 0) {
                    offset = 0 - self.boxes[i].redSave;
                }
                self.boxes[i].blue = self.boxes[i].blueSave + offset;
                self.boxes[i].green = self.boxes[i].greenSave + offset;
                self.boxes[i].red = self.boxes[i].redSave + offset;
                self.boxes[i].greenSave = self.boxes[i].green;
                self.boxes[i].redSave = self.boxes[i].red;
                self.boxes[i].blueSave = self.boxes[i].blue;
            }
        }
    }
}

void export(const char *filename) {
    self.saved = 1;
    FILE *fp = fopen(filename, "w");
    char messages[][64] = {
        "// text color (0)",
        "// text color alternate (3)",
        "// ribbon top bar color (6)",
        "// ribbon dropdown color (9)",
        "// ribbon select color (12)",
        "// popup box color (15)",
        "// popup boxes color (18)",
        "// popup boxes select color (21)",
        "// button color (24)",
        "// button select color (27)",
        "// switch color off (30)",
        "// switch circle color off (33)",
        "// switch color on (36)",
        "// switch circle color on (39)",
        "// dial color (42)",
        "// dial inner circle color (45)",
        "// slider bar color (48)",
        "// slider circle color (51)",
        "// dropdown color (54)",
        "// dropdown select color (57)",
        "// dropdown hover color (60)",
        "// dropdown triangle color (63)",
    };
    for (uint32_t i = 0; i < NUMBER_OF_BOXES - 1; i++) {
        char line[128];
        sprintf(line, "%0.1lf, %0.1lf, %0.1lf,", self.boxes[i + 1].red, self.boxes[i + 1].green, self.boxes[i + 1].blue);
        while (strlen(line) < 21) {
            strcat(line, " ");
        }
        strcat(line, messages[i]);
        strcat(line, "\n");
        fwrite(line, strlen(line), 1, fp);
    }
    fclose(fp);
}

void import(const char *filename) {
    self.saved = -1;
    FILE *fp = fopen(filename, "r");
    for (uint32_t i = 0; i < NUMBER_OF_BOXES - 1; i++) {
        char line[128];
        fgets(line, 128, fp);
        sscanf(line, "%lf, %lf, %lf", &self.boxes[i + 1].red, &self.boxes[i + 1].green, &self.boxes[i + 1].blue);
    }
    /* special - make background color the same as TT_COLOR_DIAL_INNER */
    self.boxes[0].red = self.boxes[16].red;
    self.boxes[0].green = self.boxes[16].green;
    self.boxes[0].blue = self.boxes[16].blue;
    fclose(fp);
}

void parseRibbonOutput() {
    if (ribbonRender.output[0] == 1) {
        ribbonRender.output[0] = 0;
        if (ribbonRender.output[1] == 0) { // File
            if (ribbonRender.output[2] == 1) { // New
                strcpy(osToolsFileDialog.selectedFilename, "null");
                self.saved = -1;
                for (uint32_t i = 0; i < NUMBER_OF_BOXES; i++) {
                    self.boxes[i].red = randomDouble(0, 255);
                    self.boxes[i].green = randomDouble(0, 255);
                    self.boxes[i].blue = randomDouble(0, 255);
                    self.newColorPalette[i * 6 + 0] = self.boxes[i].red;
                    self.newColorPalette[i * 6 + 1] = self.boxes[i].redSave;
                    self.newColorPalette[i * 6 + 2] = self.boxes[i].green;
                    self.newColorPalette[i * 6 + 3] = self.boxes[i].greenSave;
                    self.newColorPalette[i * 6 + 4] = self.boxes[i].blue;
                    self.newColorPalette[i * 6 + 5] = self.boxes[i].blueSave;
                }
                list_clear(self.undoList);
                self.undoIndex = 0;
            }
            if (ribbonRender.output[2] == 2) { // Save
                if (strcmp(osToolsFileDialog.selectedFilename, "null") == 0) {
                    if (osToolsFileDialogPrompt(1, "") != -1) {
                        printf("Saved to: %s\n", osToolsFileDialog.selectedFilename);
                        export(osToolsFileDialog.selectedFilename);
                    }
                } else {
                    printf("Saved to: %s\n", osToolsFileDialog.selectedFilename);
                    export(osToolsFileDialog.selectedFilename);
                }
            }
            if (ribbonRender.output[2] == 3) { // Save As...
                if (osToolsFileDialogPrompt(1, "") != -1) {
                    printf("Saved to: %s\n", osToolsFileDialog.selectedFilename);
                    export(osToolsFileDialog.selectedFilename);
                }
            }
            if (ribbonRender.output[2] == 4) { // Open
                if (osToolsFileDialogPrompt(0, "") != -1) {
                    printf("Loaded data from: %s\n", osToolsFileDialog.selectedFilename);
                    import(osToolsFileDialog.selectedFilename);
                    list_clear(self.undoList);
                    self.undoIndex = 0;
                    self.saved = -1;
                    for (uint32_t i = 0; i < NUMBER_OF_BOXES; i++) {
                        self.newColorPalette[i * 6 + 0] = self.boxes[i].red;
                        self.newColorPalette[i * 6 + 1] = self.boxes[i].redSave;
                        self.newColorPalette[i * 6 + 2] = self.boxes[i].green;
                        self.newColorPalette[i * 6 + 3] = self.boxes[i].greenSave;
                        self.newColorPalette[i * 6 + 4] = self.boxes[i].blue;
                        self.newColorPalette[i * 6 + 5] = self.boxes[i].blueSave;
                    }
                }
            }
        }
        if (ribbonRender.output[1] == 1) { // Edit
            if (ribbonRender.output[2] == 1) { // Undo
                undo();
            }
            if (ribbonRender.output[2] == 2) { // Redo
                redo();
            }
            if (ribbonRender.output[2] == 3) { // Cut
                osToolsClipboardSetText("test123");
                printf("Cut \"test123\" to clipboard!\n");
            }
            if (ribbonRender.output[2] == 4) { // Copy
                osToolsClipboardSetText("test345");
                printf("Copied \"test345\" to clipboard!\n");
            }
            if (ribbonRender.output[2] == 5) { // Paste
                osToolsClipboardGetText();
                printf("Pasted \"%s\" from clipboard!\n", osToolsClipboard.text);
            }
        }
        if (ribbonRender.output[1] == 2) { // View
            if (ribbonRender.output[2] == 1) { // Change theme
                printf("Change theme\n");
                if (tt_theme == TT_THEME_DARK) {
                    turtleBgColor(180, 180, 180);
                    turtleToolsLightTheme();
                } else {
                    turtleBgColor(30, 30, 30);
                    turtleToolsDarkTheme();
                }
            } 
            if (ribbonRender.output[2] == 2) { // GLFW
                printf("GLFW settings\n");
            } 
        }
    }
}

void parsePopupOutput(GLFWwindow *window) {
    if (popup.output[0] == 1) {
        popup.output[0] = 0; // untoggle
        if (popup.output[1] == 0) { // save
            if (strcmp(osToolsFileDialog.selectedFilename, "null") == 0) {
                if (osToolsFileDialogPrompt(1, "") != -1) {
                    printf("Saved to: %s\n", osToolsFileDialog.selectedFilename);
                    export(osToolsFileDialog.selectedFilename);
                }
            } else {
                printf("Saved to: %s\n", osToolsFileDialog.selectedFilename);
                export(osToolsFileDialog.selectedFilename);
            }
            turtle.close = 0;
            glfwSetWindowShouldClose(window, 0);
        }
        if (popup.output[1] == 1) { // cancel
            turtle.close = 0;
            glfwSetWindowShouldClose(window, 0);
        }
        if (popup.output[1] == 2) { // close
            turtle.shouldClose = 1;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Initialize glfw */
    if (!glfwInit()) {
        return -1;
    }
    glfwWindowHint(GLFW_SAMPLES, 4); // MSAA (Anti-Aliasing) with 4 samples (must be done before window is created (?))

    /* Create a windowed mode window and its OpenGL context */
    const GLFWvidmode *monitorSize = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int32_t windowHeight = monitorSize -> height * 0.85;
    GLFWwindow *window = glfwCreateWindow(windowHeight * 16 / 9, windowHeight, "palette", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeLimits(window, windowHeight * 16 / 9, windowHeight, windowHeight * 16 / 9, windowHeight);
    char constructedPath[4097 + 32];

    /* initialise osTools */
    osToolsInit(argv[0], window); // must include argv[0] to get executableFilepath, must include GLFW window
    osToolsFileDialogAddExtension("txt"); // add txt to extension restrictions
    osToolsFileDialogAddExtension("pal"); // add pal to extension restrictions
    /* initialize turtle */
    turtleInit(window, -320, -180, 320, 180);
    /* initialise turtleText */
    strcpy(constructedPath, osToolsFileDialog.executableFilepath);
    strcat(constructedPath, "include/fontBez.tgl");
    turtleTextInit(constructedPath);
    /* initialise turtleTools ribbon */
    strcpy(constructedPath, osToolsFileDialog.executableFilepath);
    strcat(constructedPath, "include/ribbonConfig.txt");
    ribbonInit(constructedPath);
    /* initialise turtleTools popup */
    strcpy(constructedPath, osToolsFileDialog.executableFilepath);
    strcat(constructedPath, "include/popupConfig.txt");
    popupInit(constructedPath, -60, -20, 60, 20);

    uint32_t tps = 120; // ticks per second (locked to fps in this case)
    uint64_t tick = 0; // count number of ticks since application started
    clock_t start, end;

    turtleBgColor(180, 180, 180);

    init();
    if (argc > 1) {
        strcpy(osToolsFileDialog.selectedFilename, argv[1]);
        import(osToolsFileDialog.selectedFilename);
    }


    while (turtle.shouldClose == 0) {
        start = clock();
        turtleGetMouseCoords();
        turtleClear();
        renderBoxes();
        char coordsStr[24];
        sprintf(coordsStr, "%.2lf, %.2lf", turtle.mouseX, turtle.mouseY);
        tt_setColor(TT_COLOR_TEXT);
        turtleTextWriteString(coordsStr, -310, -170, 5, 0);
        turtleToolsUpdateUI(); // update turtleTools UI elements
        renderFakePopup();
        mouseTick();
        /* override popup if saved */
        if (turtle.close == 1 && self.saved == 1) {
            turtle.shouldClose = 1;
            turtleFree();
            glfwTerminate();
            return 0;
        }
        turtleToolsUpdateRibbonPopup(); // update turtleTools ribbon and popup
        displaySaveIndicator();
        parseRibbonOutput(); // user defined function to use ribbon
        parsePopupOutput(window); // user defined function to use popup
        turtleUpdate(); // update the screen
        end = clock();
        while ((double) (end - start) / CLOCKS_PER_SEC < (1.0 / tps)) {
            end = clock();
        }
        tick++;
    }
    turtleFree();
    glfwTerminate();
    return 0;
}