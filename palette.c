#include "include/turtleTools.h"
#include "include/osTools.h"
#include <time.h>

#define NUMBER_OF_BOXES 23

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
} box_t;

/* global state */
typedef struct {
    uint32_t width;
    double boxSize;
    double topX;
    double topY;
    box_t boxes[NUMBER_OF_BOXES];
    char lockOffsets;
    char asciiEnum[NUMBER_OF_BOXES][32];
} palette_t;

palette_t self;

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
    sliderInit("slider", &sliderVar, TT_SLIDER_VERTICAL, TT_SLIDER_ALIGN_CENTER, UIX - 50, UIY - 120, 10, 50, 0, 255, 1);
    dropdownInit("dropdown", dropdownOptions, &dropdownVar, TT_DROPDOWN_ALIGN_CENTER, UIX, UIY - 165, 10);

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
        self.boxes[i].red = randomDouble(0, 255);
        self.boxes[i].green = randomDouble(0, 255);
        self.boxes[i].blue = randomDouble(0, 255);
        self.boxes[i].x = self.topX + self.boxSize * (i % self.width) * 1.05;
        self.boxes[i].y = self.topY - self.boxSize * (i / self.width) * 1.05;
        sliderInit("", &(self.boxes[i].red), TT_SLIDER_HORIZONTAL, TT_SLIDER_ALIGN_CENTER, self.boxes[i].x, self.boxes[i].y + 20, 5, self.boxSize * 0.8, 0, 255, 0);
        sliderInit("", &(self.boxes[i].green), TT_SLIDER_HORIZONTAL, TT_SLIDER_ALIGN_CENTER, self.boxes[i].x, self.boxes[i].y + 10, 5, self.boxSize * 0.8, 0, 255, 0);
        sliderInit("", &(self.boxes[i].blue), TT_SLIDER_HORIZONTAL, TT_SLIDER_ALIGN_CENTER, self.boxes[i].x, self.boxes[i].y, 5, self.boxSize * 0.8, 0, 255, 0);
    }
}

void renderBoxes() {
    /* render background */
    turtlePenColor(self.boxes[0].red, self.boxes[0].green, self.boxes[0].blue);
    turtleRectangle(self. topX + self.boxSize * self.width, -180, 320, 180);
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
        } else {
            turtlePenColor(0, 0, 0);
        }
        turtleTextWriteString(rgbStr, self.boxes[i].x, self.boxes[i].y - 10, self.boxes[i].size / 10, 50);
        turtleTextWriteString(hexStr, self.boxes[i].x, self.boxes[i].y - 20, self.boxes[i].size / 10, 50);
    }
    /* set color palette */
    double newColorPalette[NUMBER_OF_BOXES * 3];
    for (uint32_t i = 0; i < NUMBER_OF_BOXES - 1; i++) {
        newColorPalette[i * 3 + 0] = self.boxes[i + 1].red;
        newColorPalette[i * 3 + 1] = self.boxes[i + 1].green;
        newColorPalette[i * 3 + 2] = self.boxes[i + 1].blue;
    }
    memcpy(tt_themeColors, newColorPalette, sizeof(tt_themeColors));
}

void mouseTick() {
    /* mouse */
    for (uint32_t i = 0; i < NUMBER_OF_BOXES; i++) {
        if (self.boxes[i].red + self.boxes[i].green + self.boxes[i].blue < 150) {
            turtlePenColor(255, 255, 255);
        } else {
            turtlePenColor(0, 0, 0);
        }
        if (turtle.mouseX > self.boxes[i].x - self.boxes[i].size * 0.5 && turtle.mouseX < self.boxes[i].x + self.boxes[i].size * 0.5 &&
            turtle.mouseY > self.boxes[i].y - self.boxes[i].size * 0.5 && turtle.mouseY < self.boxes[i].y + self.boxes[i].size * 0.5) {
            turtleTextWriteString(self.asciiEnum[i], turtle.mouseX, turtle.mouseY + 4, 8, 0);
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
        sprintf(line, "%0.1lf, %0.1lf, %0.1lf,", self.boxes[i + 1].red, self.boxes[i + 1].green, self.boxes[i + 2].blue);
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
    FILE *fp = fopen(filename, "r");
    for (uint32_t i = 0; i < NUMBER_OF_BOXES - 1; i++) {
        char line[128];
        fgets(line, 128, fp);
        sscanf(line, "%lf, %lf, %lf", &self.boxes[i + 1].red, &self.boxes[i + 1].green, &self.boxes[i + 1].blue);
    }
    fclose(fp);
}

void parseRibbonOutput() {
    if (ribbonRender.output[0] == 1) {
        ribbonRender.output[0] = 0;
        if (ribbonRender.output[1] == 0) { // File
            if (ribbonRender.output[2] == 1) { // New
                strcpy(osFileDialog.selectedFilename, "null");
                for (uint32_t i = 0; i < NUMBER_OF_BOXES; i++) {
                    self.boxes[i].red = randomDouble(0, 255);
                    self.boxes[i].green = randomDouble(0, 255);
                    self.boxes[i].blue = randomDouble(0, 255);
                }
            }
            if (ribbonRender.output[2] == 2) { // Save
                if (strcmp(osFileDialog.selectedFilename, "null") == 0) {
                    if (osFileDialogPrompt(1, "") != -1) {
                        printf("Saved to: %s\n", osFileDialog.selectedFilename);
                        export(osFileDialog.selectedFilename);
                    }
                } else {
                    printf("Saved to: %s\n", osFileDialog.selectedFilename);
                    export(osFileDialog.selectedFilename);
                }
            }
            if (ribbonRender.output[2] == 3) { // Save As...
                if (osFileDialogPrompt(1, "") != -1) {
                    printf("Saved to: %s\n", osFileDialog.selectedFilename);
                    export(osFileDialog.selectedFilename);
                }
            }
            if (ribbonRender.output[2] == 4) { // Open
                if (osFileDialogPrompt(0, "") != -1) {
                    printf("Loaded data from: %s\n", osFileDialog.selectedFilename);
                    import(osFileDialog.selectedFilename);
                }
            }
        }
        if (ribbonRender.output[1] == 1) { // Edit
            if (ribbonRender.output[2] == 1) { // Undo
                printf("Undo\n");
            }
            if (ribbonRender.output[2] == 2) { // Redo
                printf("Redo\n");
            }
            if (ribbonRender.output[2] == 3) { // Cut
                osClipboardSetText("test123");
                printf("Cut \"test123\" to clipboard!\n");
            }
            if (ribbonRender.output[2] == 4) { // Copy
                osClipboardSetText("test345");
                printf("Copied \"test345\" to clipboard!\n");
            }
            if (ribbonRender.output[2] == 5) { // Paste
                osClipboardGetText();
                printf("Pasted \"%s\" from clipboard!\n", osClipboard.text);
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
        if (popup.output[1] == 0) { // cancel
            turtle.close = 0;
            glfwSetWindowShouldClose(window, 0);
        }
        if (popup.output[1] == 1) { // close
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
    GLFWwindow *window = glfwCreateWindow(windowHeight * 16 / 9, windowHeight, "turtle", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeLimits(window, windowHeight * 16 / 9, windowHeight, windowHeight * 16 / 9, windowHeight);

    /* initialize turtle */
    turtleInit(window, -320, -180, 320, 180);
    /* initialise turtleText */
    turtleTextInit("include/fontBez.tgl");
    /* initialise turtleTools ribbon */
    ribbonInit("include/ribbonConfig.txt");
    /* initialise turtleTools popup */
    popupInit("include/popupConfig.txt", -60, -20, 60, 20);
    /* initialise osTools */
    osToolsInit(argv[0], window); // must include argv[0] to get executableFilepath, must include GLFW window
    osFileDialogAddExtension("txt"); // add txt to extension restrictions

    uint32_t tps = 120; // ticks per second (locked to fps in this case)
    uint64_t tick = 0; // count number of ticks since application started
    clock_t start, end;

    turtleBgColor(180, 180, 180);

    init();
    if (argc > 1) {
        import(argv[1]);
    }

    while (turtle.shouldClose == 0) {
        start = clock();
        turtleGetMouseCoords();
        turtleClear();
        renderBoxes();
        char coordsStr[16];
        sprintf(coordsStr, "%.2lf, %.2lf", turtle.mouseX, turtle.mouseY);
        tt_setColor(TT_COLOR_TEXT);
        turtleTextWriteString(coordsStr, -310, -170, 5, 0);
        turtleToolsUpdate(); // update turtleTools
        mouseTick();
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