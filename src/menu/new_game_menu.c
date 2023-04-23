#include "new_game_menu.h"

#include "../font/font.h"
#include "../font/dejavusans.h"

#include "../graphics/image.h"
#include "../util/memory.h"
#include "../util/rom.h"

#include "../build/assets/materials/ui.h"
#include "../build/assets/materials/images.h"

#include "../levels/levels.h"
#include "../controls/controller.h"

struct Chapter gChapters[] = {
    {"CHAPTER 1", "Testchamber 00", images_chapter1_rgba_16b, 0},
    {"CHAPTER 2", "Testchamber 04", images_chapter2_rgba_16b, 2},
    {"CHAPTER 3", "Testchamber 08", images_chapter3_rgba_16b, 4},
    {"CHAPTER 4", "Testchamber 10", images_chapter4_rgba_16b, 6},
    {"CHAPTER 5", "Testchamber 13", images_chapter5_rgba_16b, -1},
    {"CHAPTER 6", "Testchamber 14", images_chapter6_rgba_16b, -1},
    {"CHAPTER 7", "Testchamber 15", images_chapter7_rgba_16b, -1},
    {"CHAPTER 8", "Testchamber 16", images_chapter8_rgba_16b, -1},
    {"CHAPTER 9", "Testchamber 17", images_chapter9_rgba_16b, -1},
    {"CHAPTER 10", "Testchamber 18", images_chapter10_rgba_16b, -1},
    {"CHAPTER 11", "Testchamber 19", images_chapter11_rgba_16b, -1},
    {NULL, NULL, NULL},
};

#define CHAPTER_COUNT   (sizeof(gChapters) / sizeof(*gChapters))

#define CHAPTER_IMAGE_SIZE  (84 * 48 * 2)

void chapterMenuInit(struct ChapterMenu* chapterMenu, int x, int y) {
    chapterMenu->chapterText = malloc(sizeof(Gfx) * 3 * 10 + 1);
    chapterMenu->testChamberText = malloc(sizeof(Gfx) * 3 * 14 + 1);
    chapterMenu->border = menuBuildSolidBorder(
        x, y + 27, 
        92, 58,
        x + 5, y + 32,
        84, 48
    );

    chapterMenu->imageBuffer = malloc(CHAPTER_IMAGE_SIZE);

    zeroMemory(chapterMenu->imageBuffer, CHAPTER_IMAGE_SIZE);

    Gfx* dl = chapterMenu->chapterText;
    gSPEndDisplayList(dl++);

    dl = chapterMenu->testChamberText;
    gSPEndDisplayList(dl++);

    chapterMenu->chapter = NULL;

    chapterMenu->x = x;
    chapterMenu->y = y;
}

void chapterMenuSetChapter(struct ChapterMenu* chapterMenu, struct Chapter* chapter) {
    Gfx* dl;
    
    if (chapter->chapter) {
        dl = fontRender(
            &gDejaVuSansFont, 
            chapter->chapter, 
            chapterMenu->x, 
            chapterMenu->y, 
            chapterMenu->chapterText
        );
        gSPEndDisplayList(dl++);
    }

    if (chapter->testChamber) {
        dl = fontRender(
            &gDejaVuSansFont, 
            chapter->testChamber, 
            chapterMenu->x, 
            chapterMenu->y + 14, 
            chapterMenu->testChamberText
        );
        gSPEndDisplayList(dl++);
    }

    if (chapter->imageData) {
        romCopy(chapter->imageData, chapterMenu->imageBuffer, CHAPTER_IMAGE_SIZE);
    }

    chapterMenu->chapter = chapter;
}

#define NEW_GAME_LEFT       40
#define NEW_GAME_TOP        45

void newGameInit(struct NewGameMenu* newGameMenu) {
    newGameMenu->menuOutline = menuBuildBorder(NEW_GAME_LEFT, NEW_GAME_TOP, SCREEN_WD - NEW_GAME_LEFT * 2, SCREEN_HT - NEW_GAME_TOP * 2);

    newGameMenu->newGameText = menuBuildText(&gDejaVuSansFont, "NEW GAME", 48, 48);
    
    newGameMenu->topLine = menuBuildHorizontalLine(52, 64, 214);
    newGameMenu->bottomLine = menuBuildHorizontalLine(52, 162, 214);

    newGameMenu->cancelButton = menuBuildButton(&gDejaVuSansFont, "Cancel", 218, 169, 46, 16);

    chapterMenuInit(&newGameMenu->chapter0, 55, 71);
    chapterMenuInit(&newGameMenu->chapter1, 163, 71);

    chapterMenuSetChapter(&newGameMenu->chapter0, &gChapters[0]);
    chapterMenuSetChapter(&newGameMenu->chapter1, &gChapters[1]);

    newGameMenu->chapterOffset = 0;
    newGameMenu->selectedChapter = 0;
}

enum MainMenuState newGameUpdate(struct NewGameMenu* newGameMenu) {
    if (controllerGetButtonDown(0, B_BUTTON)) {
        return MainMenuStateLanding;
    }

    if (controllerGetButtonDown(0, A_BUTTON) && gChapters[newGameMenu->selectedChapter + 1].testChamberNumber >= 0) {
        struct Transform identityTransform;
        transformInitIdentity(&identityTransform);
        identityTransform.position.y = 1.0f;

        levelQueueLoad(gChapters[newGameMenu->selectedChapter].testChamberNumber, &identityTransform, &gZeroVec);
    }

    if ((controllerGetDirectionDown(0) & ControllerDirectionRight) != 0 && 
        newGameMenu->selectedChapter + 1 < CHAPTER_COUNT &&
        gChapters[newGameMenu->selectedChapter + 1].imageData) {
        newGameMenu->selectedChapter = newGameMenu->selectedChapter + 1;
    }

    if ((controllerGetDirectionDown(0) & ControllerDirectionLeft) != 0 && newGameMenu->selectedChapter > 0) {
        newGameMenu->selectedChapter = newGameMenu->selectedChapter - 1;
    }

    int nextChapterOffset = newGameMenu->selectedChapter & ~1;

    if (nextChapterOffset != newGameMenu->chapterOffset) {
        newGameMenu->chapterOffset = nextChapterOffset;

        chapterMenuSetChapter(&newGameMenu->chapter0, &gChapters[newGameMenu->chapterOffset]);
        chapterMenuSetChapter(&newGameMenu->chapter1, &gChapters[newGameMenu->chapterOffset + 1]);
    }

    return MainMenuStateNewGame;
}

void newGameRender(struct NewGameMenu* newGameMenu, struct RenderState* renderState, struct GraphicsTask* task) {
    gSPDisplayList(renderState->dl++, ui_material_list[DEFAULT_UI_INDEX]);

    gSPDisplayList(renderState->dl++, ui_material_list[SOLID_TRANSPARENT_OVERLAY_INDEX]);
    gDPFillRectangle(renderState->dl++, 0, 0, SCREEN_WD, SCREEN_HT);
    gSPDisplayList(renderState->dl++, ui_material_revert_list[SOLID_TRANSPARENT_OVERLAY_INDEX]);

    gSPDisplayList(renderState->dl++, ui_material_list[ROUNDED_CORNERS_INDEX]);
    gSPDisplayList(renderState->dl++, newGameMenu->menuOutline);
    gSPDisplayList(renderState->dl++, ui_material_revert_list[ROUNDED_CORNERS_INDEX]);

    gSPDisplayList(renderState->dl++, ui_material_list[SOLID_ENV_INDEX]);
    gSPDisplayList(renderState->dl++, newGameMenu->topLine);
    gSPDisplayList(renderState->dl++, newGameMenu->bottomLine);
    gSPDisplayList(renderState->dl++, newGameMenu->cancelButton.outline);

    gDPPipeSync(renderState->dl++);
    menuSetRenderColor(renderState, newGameMenu->selectedChapter == newGameMenu->chapterOffset, &gSelectionOrange, &gColorBlack);
    gSPDisplayList(renderState->dl++, newGameMenu->chapter0.border);

    if (newGameMenu->chapter1.chapter->imageData) {
        gDPPipeSync(renderState->dl++);
        menuSetRenderColor(renderState, newGameMenu->selectedChapter != newGameMenu->chapterOffset, &gSelectionOrange, &gColorBlack);
        gSPDisplayList(renderState->dl++, newGameMenu->chapter1.border);
    }

    gSPDisplayList(renderState->dl++, ui_material_revert_list[SOLID_ENV_INDEX]);

    gSPDisplayList(renderState->dl++, ui_material_list[DEJAVU_SANS_INDEX]);
    gSPDisplayList(renderState->dl++, newGameMenu->newGameText);
    gSPDisplayList(renderState->dl++, newGameMenu->cancelButton.text);

    gDPPipeSync(renderState->dl++);
    menuSetRenderColor(renderState, newGameMenu->selectedChapter == newGameMenu->chapterOffset, &gSelectionOrange, &gColorWhite);
    gSPDisplayList(renderState->dl++, newGameMenu->chapter0.chapterText);
    gSPDisplayList(renderState->dl++, newGameMenu->chapter0.testChamberText);

    if (newGameMenu->chapter1.chapter->imageData) {
        gDPPipeSync(renderState->dl++);
        menuSetRenderColor(renderState, newGameMenu->selectedChapter != newGameMenu->chapterOffset, &gSelectionOrange, &gColorWhite);
        gSPDisplayList(renderState->dl++, newGameMenu->chapter1.chapterText);
        gSPDisplayList(renderState->dl++, newGameMenu->chapter1.testChamberText);
        gSPDisplayList(renderState->dl++, ui_material_revert_list[DEJAVU_SANS_INDEX]);
    }

    graphicsCopyImage(
        renderState, newGameMenu->chapter0.imageBuffer, 
        84, 48, 
        0, 0, 
        newGameMenu->chapter0.x + 5,
        newGameMenu->chapter0.y + 32,
        84, 48,
        gColorWhite
    );

    if (newGameMenu->chapter1.chapter->imageData) {
        graphicsCopyImage(
            renderState, newGameMenu->chapter1.imageBuffer, 
            84, 48, 
            0, 0, 
            newGameMenu->chapter1.x + 5,
            newGameMenu->chapter1.y + 32,
            84, 48,
            gColorWhite
        );
    }
}