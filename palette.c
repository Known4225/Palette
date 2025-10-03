/* how to edit for new colors:
- Edit turtleTools.h file with new changes
- Change NUMBER_OF_BOXES
- Add relevant names to asciiCopy in init function
- Change messages in export function
- 
*/

#include "turtle.h"
#include <time.h>

#define NUMBER_OF_BOXES 42
#define NUMBER_OF_COLUMNS 7
#define BOX_SIZE 40

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
    char asciiEnum[NUMBER_OF_BOXES][64];
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
    list_t *yPositions;
    tt_context_t *context;
    double scroll; // scroll wheel
    double scrollFactor; // scroll wheel percentage of scrollbar advanced per scroll
} palette_t;

palette_t self;
tt_popup_t fakePopup;

/* UI variables */
int8_t buttonVar = 0, switchVar = 0, checkboxVar = 0;
int32_t dropdownVar = 0, dropdownVar2 = 0;
double dialVar = 0.0, sliderVar = 0.0, scrollbarVar = 0.0;

void init() {
    /* setup UI */
    list_t *dropdownOptions = list_init();
    list_append(dropdownOptions, (unitype) "dropA", 's');
    list_append(dropdownOptions, (unitype) "dropB", 's');
    list_append(dropdownOptions, (unitype) "dropC", 's');
    list_append(dropdownOptions, (unitype) "dropD", 's');
    list_append(dropdownOptions, (unitype) "dropE", 's');
    list_t *dropdownOptions2 = list_init();
    list_append(dropdownOptions2, (unitype) "Fotran", 's');
    list_append(dropdownOptions2, (unitype) "C", 's');
    list_append(dropdownOptions2, (unitype) "C++", 's');
    list_append(dropdownOptions2, (unitype) "Ada", 's');
    list_append(dropdownOptions2, (unitype) "Go", 's');
    double UIX = 200;
    double UIY = 100;
    buttonInit("Button", &buttonVar, UIX, UIY, 10);
    switchInit("Switch", &switchVar, UIX, UIY - 40, 10);
    tt_switch_t *checkbox = switchInit("Checkbox", &checkboxVar, UIX - 30, UIY - 220, 10);
    checkbox -> style = TT_SWITCH_STYLE_CHECKBOX;
    dialInit("Dial", &dialVar, TT_DIAL_EXP, UIX, UIY - 80, 10, 0, 1000, 1);
    sliderInit("Slider", &sliderVar, TT_SLIDER_HORIZONTAL, TT_SLIDER_ALIGN_CENTER, UIX, UIY - 120, 10, 50, 0, 10, 1);
    sliderInit("Slider", &sliderVar, TT_SLIDER_VERTICAL, TT_SLIDER_ALIGN_CENTER, UIX - 50, UIY - 120, 10, 50, 0, 10, 1);
    scrollbarInit(&scrollbarVar, TT_SCROLLBAR_VERTICAL, 310, 0, 10, 320, 90);
    dropdownInit("Dropdown", dropdownOptions, &dropdownVar, TT_DROPDOWN_ALIGN_CENTER, UIX, UIY - 200, 10);
    dropdownInit("Dropdown", dropdownOptions2, &dropdownVar2, TT_DROPDOWN_ALIGN_CENTER, UIX, UIY - 165, 10);
    textboxInit("textbox", 128, 150, 130, 10, 100);
    list_t *contextOptions = list_init();
    list_append(contextOptions, (unitype) "Hello", 's');
    list_append(contextOptions, (unitype) "World", 's');
    list_append(contextOptions, (unitype) "Test", 's');
    int32_t contextVar = 0;
    self.context = contextInit(contextOptions, &contextVar, 0, 0, 10);
    self.context -> enabled = TT_ELEMENT_HIDE;
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
    list_append(fakePopup.options, (unitype) "Yes", 's');
    list_append(fakePopup.options, (unitype) "No", 's');

    /* setup asciiEnum */
    for (uint32_t i = 0; i < NUMBER_OF_BOXES; i++) {
        strcpy(self.asciiEnum[i], "NULL");
    }
    char asciiCopy[][64] = {
        "TT_COLOR_BACKGROUND",
        "TT_COLOR_BACKGROUND_HIGHLIGHT",
        "TT_COLOR_BACKGROUND_ALTERNATE",
        "TT_COLOR_BACKGROUND_COMPLEMENT",
        "TT_COLOR_TEXT",
        "TT_COLOR_TEXT_HIGHLIGHT",
        "TT_COLOR_TEXT_ALTERNATE",
        "TT_COLOR_TEXT_COMPLEMENT",
        "TT_COLOR_COMPONENT",
        "TT_COLOR_COMPONENT_HIGHLIGHT",
        "TT_COLOR_COMPONENT_ALTERNATE",
        "TT_COLOR_COMPONENT_COMPLEMENT",
        "TT_COLOR_TERTIARY",
        "TT_COLOR_TERTIARY_HIGHLIGHT",
        "TT_COLOR_TERTIARY_ALTERNATE",
        "TT_COLOR_TERTIARY_COMPLEMENT",
        "TT_COLOR_RED",
        "TT_COLOR_RED_ALTERNATE",
        "TT_COLOR_ORANGE",
        "TT_COLOR_ORANGE_ALTERNATE",
        "TT_COLOR_YELLOW",
        "TT_COLOR_YELLOW_ALTERNATE",
        "TT_COLOR_GREEN",
        "TT_COLOR_GREEN_ALTERNATE",
        "TT_COLOR_CYAN",
        "TT_COLOR_CYAN_ALTERNATE",
        "TT_COLOR_BLUE",
        "TT_COLOR_BLUE_ALTERNATE",
        "TT_COLOR_PURPLE",
        "TT_COLOR_PURPLE_ALTERNATE",
        "TT_COLOR_MAGENTA",
        "TT_COLOR_MAGENTA_ALTERNATE",
        "TT_COLOR_PINK",
        "TT_COLOR_PINK_ALTERNATE",
        "TT_COLOR_BLACK",
        "TT_COLOR_BLACK_ALTERNATE",
        "TT_COLOR_WHITE",
        "TT_COLOR_WHITE_ALTERNATE",
        "TT_COLOR_DARK_GREY",
        "TT_COLOR_DARK_GREY_ALTERNATE",
        "TT_COLOR_LIGHT_GREY",
        "TT_COLOR_LIGHT_GREY_ALTERNATE",
    };
    for (uint32_t i = 0; i < sizeof(asciiCopy) / 64; i++) {
        strcpy(self.asciiEnum[i], asciiCopy[i]);
    }

    /* boxes */
    self.lockOffsets = 0;
    self.width = NUMBER_OF_COLUMNS;
    self.boxSize = BOX_SIZE;
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
        self.boxes[i].sliders[0] = sliderInit("", &(self.boxes[i].red), TT_SLIDER_HORIZONTAL, TT_SLIDER_ALIGN_CENTER, self.boxes[i].x, self.boxes[i].y + self.boxSize / 2.5, 5, self.boxSize * 0.8, 0, 255, 0);
        self.boxes[i].sliders[1] = sliderInit("", &(self.boxes[i].green), TT_SLIDER_HORIZONTAL, TT_SLIDER_ALIGN_CENTER, self.boxes[i].x, self.boxes[i].y + self.boxSize / 5, 5, self.boxSize * 0.8, 0, 255, 0);
        self.boxes[i].sliders[2] = sliderInit("", &(self.boxes[i].blue), TT_SLIDER_HORIZONTAL, TT_SLIDER_ALIGN_CENTER, self.boxes[i].x, self.boxes[i].y, 5, self.boxSize * 0.8, 0, 255, 0);
        for (int32_t j = 0; j < 3; j++) {
            self.boxes[i].sliders[j] -> color[TT_COLOR_SLOT_SLIDER_BAR] = TT_COLOR_BLACK;
            self.boxes[i].sliders[j] -> color[TT_COLOR_SLOT_SLIDER_CIRCLE] = TT_COLOR_WHITE;
        }
    }

    self.undoList = list_init();
    self.undoIndex = 0;
    self.copyMessage = 0;
    self.dragBox = -1;
    self.saved = 1;
    self.firstUndo = 1;
    self.firstRedo = 1;
    memset(self.keys, 0, sizeof(self.keys));

    self.scroll = 0.0;
    self.scrollFactor = 15;
    self.yPositions = list_init();
    for (uint32_t i = 0; i < tt_elements.all -> length; i++) {
        list_append(self.yPositions, (unitype) ((tt_button_t *) tt_elements.all -> data[i].p) -> y, 'd');
    }
    list_append(self.yPositions, (unitype) fakePopup.minY, 'd');
    list_append(self.yPositions, (unitype) fakePopup.maxY, 'd');
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
    // printf("add to undo %d\n", self.undoIndex);
    self.saved = 0;
}

void undo() {
    // printf("undo %d\n", self.undoIndex);
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
            for (int32_t j = 0; j < 3; j++) {
                self.boxes[i].sliders[j] -> color[TT_COLOR_SLOT_SLIDER_BAR] = TT_COLOR_WHITE;
                self.boxes[i].sliders[j] -> color[TT_COLOR_SLOT_SLIDER_CIRCLE] = TT_COLOR_BLACK;
            }
        } else {
            turtlePenColor(0, 0, 0);
            for (int32_t j = 0; j < 3; j++) {
                self.boxes[i].sliders[j] -> color[TT_COLOR_SLOT_SLIDER_BAR] = TT_COLOR_BLACK;
                self.boxes[i].sliders[j] -> color[TT_COLOR_SLOT_SLIDER_CIRCLE] = TT_COLOR_WHITE;
            }
        }
        turtleTextWriteString(rgbStr, self.boxes[i].x, self.boxes[i].y - self.boxes[i].size / 5, self.boxes[i].size / 10, 50);
        turtleTextWriteString(hexStr, self.boxes[i].x, self.boxes[i].y - self.boxes[i].size / 2.5, self.boxes[i].size / 10, 50);
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
    memcpy(tt_themeColors, setColorPalette, NUMBER_OF_BOXES * sizeof(double) * 3);
    turtleBgColor(tt_themeColors[TT_COLOR_BACKGROUND_COMPLEMENT], tt_themeColors[TT_COLOR_BACKGROUND_COMPLEMENT + 1], tt_themeColors[TT_COLOR_BACKGROUND_COMPLEMENT + 2]);
    /* display copy message */
    if (self.copyMessage > 0) {
        if (self.copyMessage == 255) {
            osToolsClipboardGetText();
        }
        self.copyMessage--;
        char copyText[64] = "copied ";
        if (osToolsClipboard.text != NULL) {
            memcpy(copyText + 7, osToolsClipboard.text, strlen(osToolsClipboard.text));
            turtlePenColorAlpha(self.boxes[1].red, self.boxes[1].green, self.boxes[1].blue, 255 - self.copyMessage);
            turtleTextWriteString(copyText, self.topX + self.boxSize * self.width + 5, 160, 8, 0);
        }
    }
}

void displaySaveIndicator() {
    /* display saved indicator */
    if (self.saved == 0) {
        turtlePenColor(self.boxes[0].red, self.boxes[0].green, self.boxes[0].blue);
        turtlePenSize(5);
        turtleGoto(310, 175);
        turtlePenDown();
        turtlePenUp();
    }
}

void renderFakePopup() {
    tt_setColor(TT_COLOR_SLOT_POPUP_BOX);
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
            tt_setColor(TT_COLOR_SLOT_POPUP_BUTTON_SELECT);
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
            tt_setColor(TT_COLOR_SLOT_POPUP_BUTTON);
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

void export(const char *filename) {
    self.saved = 1;
    FILE *fp = fopen(filename, "w");
    char messages[][64] = {
        "// TT_COLOR_BACKGROUND (0)",
        "// TT_COLOR_BACKGROUND_HIGHLIGHT (3)",
        "// TT_COLOR_BACKGROUND_ALTERNATE (6)",
        "// TT_COLOR_BACKGROUND_COMPLEMENT (9)",
        "// TT_COLOR_TEXT (12)",
        "// TT_COLOR_TEXT_HIGHLIGHT (15)",
        "// TT_COLOR_TEXT_ALTERNATE (18)",
        "// TT_COLOR_TEXT_COMPLEMENT (21)",
        "// TT_COLOR_COMPONENT (24)",
        "// TT_COLOR_COMPONENT_HIGHLIGHT (27)",
        "// TT_COLOR_COMPONENT_ALTERNATE (30)",
        "// TT_COLOR_COMPONENT_COMPLEMENT (33)",
        "// TT_COLOR_TERTIARY (36)",
        "// TT_COLOR_TERTIARY_HIGHLIGHT (39)",
        "// TT_COLOR_TERTIARY_ALTERNATE (42)",
        "// TT_COLOR_TERTIARY_COMPLEMENT (45)",
        "// TT_COLOR_RED (48)",
        "// TT_COLOR_RED_ALTERNATE (51)",
        "// TT_COLOR_ORANGE (54)",
        "// TT_COLOR_ORANGE_ALTERNATE (57)",
        "// TT_COLOR_YELLOW (60)",
        "// TT_COLOR_YELLOW_ALTERNATE (63)",
        "// TT_COLOR_GREEN (66)",
        "// TT_COLOR_GREEN_ALTERNATE (69)",
        "// TT_COLOR_CYAN (72)",
        "// TT_COLOR_CYAN_ALTERNATE (75)",
        "// TT_COLOR_BLUE (78)",
        "// TT_COLOR_BLUE_ALTERNATE (81)",
        "// TT_COLOR_PURPLE (84)",
        "// TT_COLOR_PURPLE_ALTERNATE (87)",
        "// TT_COLOR_MAGENTA (90)",
        "// TT_COLOR_MAGENTA_ALTERNATE (93)",
        "// TT_COLOR_PINK (96)",
        "// TT_COLOR_PINK_ALTERNATE (99)",
        "// TT_COLOR_BLACK (102)",
        "// TT_COLOR_BLACK_ALTERNATE (105)",
        "// TT_COLOR_WHITE (108)",
        "// TT_COLOR_WHITE_ALTERNATE (111)",
        "// TT_COLOR_DARK_GREY (114)",
        "// TT_COLOR_DARK_GREY_ALTERNATE (117)",
        "// TT_COLOR_LIGHT_GREY (120)",
        "// TT_COLOR_LIGHT_GREY_ALTERNATE (123)",
    };
    for (uint32_t i = 0; i < NUMBER_OF_BOXES; i++) {
        char line[128];
        sprintf(line, "%0.1lf, %0.1lf, %0.1lf,", self.boxes[i].red, self.boxes[i].green, self.boxes[i].blue);
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
    for (uint32_t i = 0; i < NUMBER_OF_BOXES; i++) {
        char line[128];
        fgets(line, 128, fp);
        sscanf(line, "%lf, %lf, %lf", &self.boxes[i].red, &self.boxes[i].green, &self.boxes[i].blue);
    }
    /* special - make background color the same as TT_COLOR_DIAL_INNER */
    // self.boxes[0].red = self.boxes[TT_COLOR_BACKGROUND / 3 + 1].red;
    // self.boxes[0].green = self.boxes[TT_COLOR_BACKGROUND / 3 + 1].green;
    // self.boxes[0].blue = self.boxes[TT_COLOR_BACKGROUND / 3 + 1].blue;
    fclose(fp);
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
                char hexStr[16];
                sprintf(hexStr, "#%02X%02X%02X", (int) round(self.boxes[i].red), (int) round(self.boxes[i].green), (int) round(self.boxes[i].blue));
                osToolsClipboardSetText(hexStr);
                self.dragBox = i;
                self.copyMessage = 255;
            }
        } else {
            if (turtle.mouseX > self.boxes[i].x - self.boxes[i].size * 0.5 && turtle.mouseX < self.boxes[i].x + self.boxes[i].size * 0.5 &&
                turtle.mouseY > self.boxes[i].y - self.boxes[i].size * 0.5 && turtle.mouseY < self.boxes[i].y - self.boxes[i].size * 0.1) {
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
    if (turtleMouseRight()) {
        if (self.keys[2] == 0) {
            self.keys[2] = 1;
            if (turtle.mouseX > self.topX + self.boxSize * self.width) {
                self.context -> enabled = TT_ELEMENT_ENABLED;
                self.context -> x = turtle.mouseX;
                self.context -> y = turtle.mouseY;
            }
        }
    } else {
        self.keys[2] = 0;
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
                if (osToolsFileDialog.selectedFilenames -> length == 0) {
                    if (osToolsFileDialogSave(OSTOOLS_FILE_DIALOG_FILE, "", NULL) != -1) {
                        printf("Saved to: %s\n", osToolsFileDialog.selectedFilenames -> data[0].s);
                        export(osToolsFileDialog.selectedFilenames -> data[0].s);
                    }
                } else {
                    printf("Saved to: %s\n", osToolsFileDialog.selectedFilenames -> data[0].s);
                    export(osToolsFileDialog.selectedFilenames -> data[0].s);
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
                        if (osToolsClipboard.text != NULL) {
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
    /* scroll wheel */
    for (uint32_t i = 0; i < tt_elements.all -> length; i++) {
        if (((tt_button_t *) tt_elements.all -> data[i].p) -> element != TT_ELEMENT_SCROLLBAR && ((tt_button_t *) tt_elements.all -> data[i].p) -> element != TT_ELEMENT_CONTEXT && ((tt_button_t *) tt_elements.all -> data[i].p) -> x > -10) {
            ((tt_button_t *) tt_elements.all -> data[i].p) -> y = self.yPositions -> data[i].d + scrollbarVar * 0.2;
        }
    }
    fakePopup.minY = self.yPositions -> data[tt_elements.all -> length].d + scrollbarVar * 0.2;
    fakePopup.maxY = self.yPositions -> data[tt_elements.all -> length + 1].d + scrollbarVar * 0.2;
    self.scroll = turtleMouseWheel();
    if (self.scroll != 0) {
        scrollbarVar -= self.scroll * self.scrollFactor;
        if (scrollbarVar < 0) {
            scrollbarVar = 0;
        }
        if (scrollbarVar > 100) {
            scrollbarVar = 100;
        }
    }
}

void parseRibbonOutput() {
    if (tt_ribbon.output[0] == 1) {
        tt_ribbon.output[0] = 0;
        if (tt_ribbon.output[1] == 0) { // File
            if (tt_ribbon.output[2] == 1) { // New
                list_clear(osToolsFileDialog.selectedFilenames);
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
            if (tt_ribbon.output[2] == 2) { // Save
                if (osToolsFileDialog.selectedFilenames -> length == 0) {
                    if (osToolsFileDialogSave(OSTOOLS_FILE_DIALOG_FILE, "", NULL) != -1) {
                        printf("Saved to: %s\n", osToolsFileDialog.selectedFilenames -> data[0].s);
                        export(osToolsFileDialog.selectedFilenames -> data[0].s);
                    }
                } else {
                    printf("Saved to: %s\n", osToolsFileDialog.selectedFilenames -> data[0].s);
                    export(osToolsFileDialog.selectedFilenames -> data[0].s);
                }
            }
            if (tt_ribbon.output[2] == 3) { // Save As...
                if (osToolsFileDialogSave(OSTOOLS_FILE_DIALOG_FILE, "", NULL) != -1) {
                    printf("Saved to: %s\n", osToolsFileDialog.selectedFilenames -> data[0].s);
                    export(osToolsFileDialog.selectedFilenames -> data[0].s);
                }
            }
            if (tt_ribbon.output[2] == 4) { // Open
                list_clear(osToolsFileDialog.selectedFilenames);
                if (osToolsFileDialogOpen(OSTOOLS_FILE_DIALOG_SINGLE_SELECT, OSTOOLS_FILE_DIALOG_FILE, "", NULL) != -1) {
                    printf("Loaded data from: %s\n", osToolsFileDialog.selectedFilenames -> data[0].s);
                    import(osToolsFileDialog.selectedFilenames -> data[0].s);
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
        if (tt_ribbon.output[1] == 1) { // Edit
            if (tt_ribbon.output[2] == 1) { // Undo
                undo();
            }
            if (tt_ribbon.output[2] == 2) { // Redo
                redo();
            }
            if (tt_ribbon.output[2] == 3) { // Cut
                osToolsClipboardSetText("test123");
                printf("Cut \"test123\" to clipboard!\n");
            }
            if (tt_ribbon.output[2] == 4) { // Copy
                osToolsClipboardSetText("test345");
                printf("Copied \"test345\" to clipboard!\n");
            }
            if (tt_ribbon.output[2] == 5) { // Paste
                osToolsClipboardGetText();
                printf("Pasted \"%s\" from clipboard!\n", osToolsClipboard.text);
            }
        }
        if (tt_ribbon.output[1] == 2) { // View
            if (tt_ribbon.output[2] == 1) { // GLFW
                printf("GLFW settings\n");
            } 
        }
    }
}

void parsePopupOutput(GLFWwindow *window) {
    if (tt_popup.output[0] == 1) {
        tt_popup.output[0] = 0; // untoggle
        if (tt_popup.output[1] == 0) { // save
            if (osToolsFileDialog.selectedFilenames -> length == 0) {
                if (osToolsFileDialogSave(OSTOOLS_FILE_DIALOG_FILE, "", NULL) != -1) {
                    printf("Saved to: %s\n", osToolsFileDialog.selectedFilenames -> data[0].s);
                    export(osToolsFileDialog.selectedFilenames -> data[0].s);
                }
            } else {
                printf("Saved to: %s\n", osToolsFileDialog.selectedFilenames -> data[0].s);
                export(osToolsFileDialog.selectedFilenames -> data[0].s);
            }
            turtle.close = 0;
            glfwSetWindowShouldClose(window, 0);
        }
        if (tt_popup.output[1] == 1) { // cancel
            turtle.close = 0;
            glfwSetWindowShouldClose(window, 0);
        }
        if (tt_popup.output[1] == 2) { // close
            turtle.popupClose = 1;
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
    int32_t windowHeight = monitorSize -> height;
    GLFWwindow *window = glfwCreateWindow(windowHeight * 16 / 9, windowHeight, "turtle demo", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeLimits(window, windowHeight * 16 / 9 * 0.4, windowHeight * 0.4, windowHeight * 16 / 9, windowHeight);

    char constructedPath[4097 + 32];

    /* initialise osTools */
    osToolsInit(argv[0], window); // must include argv[0] to get executableFilepath, must include GLFW window
    osToolsFileDialogAddGlobalExtension("txt"); // add txt to extension restrictions
    osToolsFileDialogAddGlobalExtension("pal"); // add pal to extension restrictions
    /* initialize turtle */
    turtleInit(window, -320, -180, 320, 180);
    glfwSetWindowSize(window, windowHeight * 16 / 9 * 0.85, monitorSize -> height * 0.85); // doing it this way ensures the window spawns in the top left of the monitor and fixes resizing limits
    /* initialise turtleText */
    strcpy(constructedPath, osToolsFileDialog.executableFilepath);
    strcat(constructedPath, "config/roberto.tgl");
    turtleTextInit(constructedPath);
    /* initialise turtleTools ribbon */
    strcpy(constructedPath, osToolsFileDialog.executableFilepath);
    strcat(constructedPath, "config/ribbonConfig.txt");
    ribbonInit(constructedPath);
    /* initialise turtleTools popup */
    strcpy(constructedPath, osToolsFileDialog.executableFilepath);
    strcat(constructedPath, "config/popupConfig.txt");
    popupInit(constructedPath);

    uint32_t tps = 120; // ticks per second (locked to fps in this case)
    uint64_t tick = 0; // count number of ticks since application started
    clock_t start, end;

    init();
    if (argc > 1) {
        list_append(osToolsFileDialog.selectedFilenames, (unitype) argv[1], 's');
        import(osToolsFileDialog.selectedFilenames -> data[0].s);
    }

    while (turtle.popupClose == 0) {
        start = clock();
        turtleGetMouseCoords();
        turtleClear();
        renderBoxes();
        char coordsStr[24];
        sprintf(coordsStr, "%.2lf, %.2lf", turtle.mouseX, turtle.mouseY);
        tt_setColor(TT_COLOR_BACKGROUND);
        turtleTextWriteString(coordsStr, -310, -170, 5, 0);
        turtleToolsUpdateUI(); // update turtleTools UI elements
        renderFakePopup();
        mouseTick();
        /* override popup if saved */
        if (turtle.close == 1 && self.saved == 1) {
            turtle.popupClose = 1;
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