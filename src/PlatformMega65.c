#include "PlatformMega65.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <mega65/memory.h>
#include <mega65/hal.h>
#include <mega65/fileio.h>

#define SCREEN_RAM      0xC000
#define GRAPHICS_DATA   0x40000L
#define COLOR_RAM       0xFF80000L
#define MOD_LOAD_ADDR   0x50000L

static Platform platformInstance;
Platform* platform = &platformInstance;

static uint8_t standard_controls[26] = {
    0x49, // I (Up)
    0x4b, // K (Down)
    0x4a, // J (Left)
    0x4c, // L (Right)
    0x57, // W (Fire Up)
    0x53, // S (Fire Down)
    0x41, // A (Fire Left)
    0x44, // D (Fire Right)
    0x01, // Cycle weapon
    0x02, // Cycle item
    0x20, // Space (Use)
    0x5a, // Z (Search)
    0x4d, // M (Move)
    0x09, // Tab (Live map)
    0x12, // Live map robots
    0x1b, // ESC (Pause)
    0x06, // F6 (Music)
    0x03, // Cheat
    0x11, // Cursor Up
    0x91, // Cursor Down
    0x9d, // Cursor Left
    0x1d, // Cursor Right
    0x20, // Space
    0x0d, // Return
    0x59, // Y
    0x4e  // N
};

static uint8_t* m65_standardControls(void)
{
    return standard_controls;
}

static void (*user_interrupt)(void) = NULL;

static void m65_setInterrupt(void (*interrupt)(void))
{
    user_interrupt = interrupt;
}

static void m65_show(void)
{
}

static int m65_framesPerSecond(void)
{
    return 50;
}

static void m65_chrout(uint8_t c)
{
    (void)c;
}

static uint8_t m65_readKeyboard(void)
{
    uint8_t key = PEEK(0xD610);
    if (key) {
        POKE(0xD610, 0);
        return key;
    }
    return 0xff;
}

static void m65_keyRepeat(void)
{
}

static void m65_clearKeyBuffer(void)
{
    while (PEEK(0xD610)) {
        POKE(0xD610, 0);
    }
}

static bool m65_isKeyOrJoystickPressed(bool gamepad)
{
    (void)gamepad;
    return (PEEK(0xD610) != 0) || ((PEEK(0xDC00) & 0x1f) != 0x1f);
}

static uint16_t m65_readJoystick(bool gamepad)
{
    uint8_t joy;
    uint16_t result = 0;
    (void)gamepad;
    joy = ~PEEK(0xDC00);
    if (joy & 0x01) result |= PlatformJoystickUp;
    if (joy & 0x02) result |= PlatformJoystickDown;
    if (joy & 0x04) result |= PlatformJoystickLeft;
    if (joy & 0x08) result |= PlatformJoystickRight;
    if (joy & 0x10) result |= PlatformJoystickRed;
    return result;
}

static void setup_viciv_screen(void)
{
    uint16_t i;
    mega65_io_enable();

    POKE(0xD054, 0x05); // 16-bit text mode, full-colour for high chars
    POKE(0xD031, 0x40); // H320, fast CPU
    POKE(0xD016, 0xC9); // Fine scroll & VIC-III H640 offset

    POKE(0xD058, 80);
    POKE(0xD059, 0);
    POKE(0xD05E, 40);

    POKE(0xD060, 0x00);
    POKE(0xD061, 0xC0);
    POKE(0xD062, 0x00);

    for (i = 0; i < 1000; i++) {
        POKE(SCREEN_RAM + i * 2 + 0, (i & 0xff));
        POKE(SCREEN_RAM + i * 2 + 1, (i >> 8));
    }

    for (i = 0; i < 2000; i += 2) {
        lpoke(COLOR_RAM + i + 0, 0x00);
        lpoke(COLOR_RAM + i + 1, 0x0E);
    }
}

static void m65_loadMap(PlatformMap map, uint8_t* destination)
{
    char filename[16];
    uint8_t fd;
    uint16_t count;
    uint32_t dest_addr;

    sprintf(filename, "level-%c", 'a' + (int)map);
    closeall();
    fd = open((char*)filename);
    if (fd != 0xff) {
        dest_addr = 0;
        while ((count = read512((uint8_t*)destination + dest_addr)) > 0) {
            dest_addr += count;
            if (dest_addr >= 8960 || count < 512) break;
        }
        close(fd);
    }
}

static uint8_t* m65_loadTileset(void)
{
    extern uint8_t tileset[];
    return tileset;
}

static uint8_t io_buf[512];

static void m65_displayImage(PlatformImage image)
{
    const char* filename;
    uint8_t fd;
    uint16_t count;
    uint32_t addr = GRAPHICS_DATA;

    switch (image) {
    case ImageIntro:
        filename = "introscreen.iff";
        break;
    case ImageGameOver:
        filename = "gameover.iff";
        break;
    case ImageGame:
    default:
        filename = "gamescreen.iff";
        break;
    }

    closeall();
    fd = open((char*)filename);
    if (fd != 0xff) {
        while ((count = read512(io_buf)) > 0) {
            lcopy((uint32_t)(uint16_t)io_buf, addr, count);
            addr += count;
            if (count < 512) break;
        }
        close(fd);
    }
}

static void m65_generateTiles(uint8_t* tileData, uint8_t* tileAttributes)
{
    (void)tileData;
    (void)tileAttributes;
}

static void m65_updateTiles(uint8_t* tileData, uint8_t* tiles, uint8_t numTiles)
{
    (void)tileData;
    (void)tiles;
    (void)numTiles;
}

static void m65_renderTile(uint8_t tile, uint16_t x, uint16_t y, uint8_t variant, bool transparent)
{
    uint8_t cx = (uint8_t)(x >> 3);
    uint8_t cy = (uint8_t)(y >> 3);
    uint16_t char_dest;
    uint8_t tx, ty;
    (void)variant;
    (void)transparent;

    for (ty = 0; ty < 3; ty++) {
        if (cy + ty >= 25) break;
        char_dest = SCREEN_RAM + ((cy + ty) * 80) + (cx * 2);
        for (tx = 0; tx < 3; tx++) {
            if (cx + tx >= 40) break;
            POKE(char_dest + tx * 2, tile);
            lpoke(COLOR_RAM + ((cy + ty) * 80) + ((cx + tx) * 2), 0x0E);
        }
    }
}

static void m65_renderTiles(uint8_t backgroundTile, uint8_t foregroundTile, uint16_t x, uint16_t y, uint8_t backgroundVariant, uint8_t foregroundVariant)
{
    (void)backgroundVariant;
    (void)foregroundVariant;
    if (foregroundTile != 0) {
        m65_renderTile(foregroundTile, x, y, foregroundVariant, true);
    } else {
        m65_renderTile(backgroundTile, x, y, backgroundVariant, false);
    }
}

static void m65_renderItem(uint8_t item, uint16_t x, uint16_t y)
{
    (void)item;
    (void)x;
    (void)y;
}

static void m65_renderKey(uint8_t key, uint16_t x, uint16_t y)
{
    (void)key;
    (void)x;
    (void)y;
}

static void m65_renderHealth(uint8_t health, uint16_t x, uint16_t y)
{
    (void)health;
    (void)x;
    (void)y;
}

static void m65_renderFace(uint8_t face, uint16_t x, uint16_t y)
{
    (void)face;
    (void)x;
    (void)y;
}

static void m65_renderLiveMap(uint8_t* map)
{
    (void)map;
}

static void m65_renderLiveMapTile(uint8_t* map, uint8_t x, uint8_t y)
{
    (void)map;
    (void)x;
    (void)y;
}

static void m65_renderLiveMapUnits(uint8_t* map, uint8_t* unitTypes, uint8_t* unitX, uint8_t* unitY, uint8_t playerColor, bool showRobots)
{
    (void)map;
    (void)unitTypes;
    (void)unitX;
    (void)unitY;
    (void)playerColor;
    (void)showRobots;
}

static void m65_showCursor(uint16_t x, uint16_t y)
{
    (void)x;
    (void)y;
}

static void m65_hideCursor(void)
{
}

static void m65_setCursorShape(PlatformCursorShape shape)
{
    (void)shape;
}

static void m65_copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height)
{
    uint8_t sx = (uint8_t)(sourceX >> 3);
    uint8_t sy = (uint8_t)(sourceY >> 3);
    uint8_t dx = (uint8_t)(destinationX >> 3);
    uint8_t dy = (uint8_t)(destinationY >> 3);
    uint8_t w  = (uint8_t)(width >> 3);
    uint8_t h  = (uint8_t)(height >> 3);
    uint8_t row;

    for (row = 0; row < h; row++) {
        if (sy + row >= 25 || dy + row >= 25) break;
        memcpy((void*)(SCREEN_RAM + (dy + row) * 80 + dx * 2),
               (void*)(SCREEN_RAM + (sy + row) * 80 + sx * 2),
               w * 2);
        lcopy(COLOR_RAM + (dy + row) * 80 + dx * 2,
              COLOR_RAM + (sy + row) * 80 + sx * 2,
              w * 2);
    }
}

static void m65_clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    uint8_t cx = (uint8_t)(x >> 3);
    uint8_t cy = (uint8_t)(y >> 3);
    uint8_t w  = (uint8_t)(width >> 3);
    uint8_t h  = (uint8_t)(height >> 3);
    uint8_t row, col;

    for (row = 0; row < h; row++) {
        if (cy + row >= 25) break;
        for (col = 0; col < w; col++) {
            if (cx + col >= 40) break;
            POKE(SCREEN_RAM + (cy + row) * 80 + (cx + col) * 2, 32);
        }
    }
}

static void m65_fillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color)
{
    uint8_t cx = (uint8_t)(x >> 3);
    uint8_t cy = (uint8_t)(y >> 3);
    uint8_t w  = (uint8_t)(width >> 3);
    uint8_t h  = (uint8_t)(height >> 3);
    uint8_t row, col;

    for (row = 0; row < h; row++) {
        if (cy + row >= 25) break;
        for (col = 0; col < w; col++) {
            if (cx + col >= 40) break;
            POKE(SCREEN_RAM + (cy + row) * 80 + (cx + col) * 2, 0xa0);
            lpoke(COLOR_RAM + (cy + row) * 80 + (cx + col) * 2, color);
        }
    }
}

static void m65_startShakeScreen(void)
{
}

static void m65_shakeScreen(void)
{
    POKE(0xD016, (PEEK(0xD016) & 0xF8) | ((PEEK(0xD012) & 0x07)));
}

static void m65_stopShakeScreen(void)
{
    POKE(0xD016, 0xC9);
}

static void m65_startFadeScreen(uint16_t color, uint16_t intensity)
{
    (void)color;
    (void)intensity;
}

static void m65_fadeScreen(uint16_t intensity, bool immediate)
{
    (void)intensity;
    (void)immediate;
}

static void m65_stopFadeScreen(void)
{
}

static void m65_writeToScreenMemory(address_t address, uint8_t value)
{
    if (address < 2000) {
        POKE(SCREEN_RAM + address, value);
    }
}

static void m65_writeToScreenMemoryColor(address_t address, uint8_t value, uint8_t color, uint8_t yOffset)
{
    (void)yOffset;
    if (address < 2000) {
        POKE(SCREEN_RAM + address, value);
        lpoke(COLOR_RAM + address, color);
    }
}

static void m65_playNote(uint8_t note)
{
    (void)note;
}

static void m65_stopNote(void)
{
    POKE(0xD720, 0);
    POKE(0xD730, 0);
    POKE(0xD740, 0);
    POKE(0xD750, 0);
}

#define MAX_INSTRUMENTS 32
static unsigned short instrument_lengths[MAX_INSTRUMENTS];
static unsigned short instrument_loopstart[MAX_INSTRUMENTS];
static unsigned short instrument_looplen[MAX_INSTRUMENTS];
static unsigned long instrument_addr[MAX_INSTRUMENTS];
static unsigned char instrument_vol[MAX_INSTRUMENTS];
static unsigned char song_pattern_list[128];
static unsigned char song_length = 0;
static unsigned char current_pattern = 0;
static unsigned char current_pattern_pos = 0;
static unsigned short song_offset = 1084;
static unsigned short tempo = 6;
static unsigned long sample_rate_divisor = 1469038L;
static bool mod_playing = false;

static void audioxbar_setcoefficient(uint8_t n, uint8_t value)
{
    POKE(0xD6F4, n);
    POKE(0xD020, PEEK(0xD020));
    POKE(0xD6F5, value);
}

static void m65_loadModule(PlatformModule module)
{
    const char* modname;
    uint8_t fd;
    uint32_t addr;
    uint16_t count;
    uint8_t i;
    uint8_t max_pattern = 0;
    uint32_t sample_data_start;

    switch (module) {
    case ModuleSoundFX:   modname = "mod.soundfx"; break;
    case ModuleIntro:     modname = "mod.metal heads"; break;
    case ModuleWin:       modname = "mod.win"; break;
    case ModuleLose:      modname = "mod.lose"; break;
    case ModuleInGame1:   modname = "mod.get psyched"; break;
    case ModuleInGame2:   modname = "mod.metallic bop"; break;
    case ModuleInGame3:   modname = "mod.robot attack"; break;
    case ModuleInGame4:
    default:              modname = "mod.rushin in"; break;
    }

    closeall();
    fd = open((char*)modname);
    if (fd == 0xff) return;

    addr = MOD_LOAD_ADDR;
    while ((count = read512(io_buf)) > 0) {
        lcopy((uint32_t)(uint16_t)io_buf, addr, count);
        addr += count;
        if (count < 512) break;
    }
    close(fd);

    for (i = 0; i < 31; i++) {
        uint8_t h0 = lpeek(MOD_LOAD_ADDR + 0x14 + i * 30 + 22);
        uint8_t h1 = lpeek(MOD_LOAD_ADDR + 0x14 + i * 30 + 23);
        instrument_lengths[i] = ((uint16_t)h0 << 9) | ((uint16_t)h1 << 1);
        instrument_vol[i] = lpeek(MOD_LOAD_ADDR + 0x14 + i * 30 + 25);
        h0 = lpeek(MOD_LOAD_ADDR + 0x14 + i * 30 + 26);
        h1 = lpeek(MOD_LOAD_ADDR + 0x14 + i * 30 + 27);
        instrument_loopstart[i] = ((uint16_t)h0 << 8) | h1;
        h0 = lpeek(MOD_LOAD_ADDR + 0x14 + i * 30 + 28);
        h1 = lpeek(MOD_LOAD_ADDR + 0x14 + i * 30 + 29);
        instrument_looplen[i] = ((uint16_t)h0 << 8) | h1;
    }

    song_length = lpeek(MOD_LOAD_ADDR + 950);
    for (i = 0; i < 128; i++) {
        song_pattern_list[i] = lpeek(MOD_LOAD_ADDR + 952 + i);
        if (i < song_length && song_pattern_list[i] > max_pattern) {
            max_pattern = song_pattern_list[i];
        }
    }

    song_offset = 1084;
    sample_data_start = MOD_LOAD_ADDR + song_offset + (uint32_t)(max_pattern + 1) * 1024L;
    for (i = 0; i < MAX_INSTRUMENTS; i++) {
        instrument_addr[i] = sample_data_start;
        sample_data_start += instrument_lengths[i];
    }

    current_pattern = song_pattern_list[0];
    current_pattern_pos = 0;
}

static void m65_playModule(PlatformModule module)
{
    m65_loadModule(module);
    mod_playing = true;
}

static void m65_pauseModule(void)
{
    mod_playing = false;
    m65_stopNote();
}

static void m65_stopModule(void)
{
    mod_playing = false;
    m65_stopNote();
}

static void m65_playSample(uint8_t sample)
{
    (void)sample;
}

static void m65_stopSample(void)
{
}

static void m65_renderFrame(bool waitForNextFrame)
{
    if (waitForNextFrame) {
        uint8_t raster = PEEK(0xD012);
        while (PEEK(0xD012) == raster);
    }
    if (user_interrupt) {
        user_interrupt();
    }
}

static void m65_waitForScreenMemoryAccess(void)
{
}

static void m65_setHighlightedMenuRow(uint16_t row)
{
    (void)row;
}

void PlatformMega65_init(void)
{
    uint16_t i;

    platformInstance.quit = false;
    *(void**)&platformInstance.standardControls = (void*)&m65_standardControls;
    *(void**)&platformInstance.setInterrupt = (void*)&m65_setInterrupt;
    *(void**)&platformInstance.show = (void*)&m65_show;
    *(void**)&platformInstance.framesPerSecond = (void*)&m65_framesPerSecond;
    *(void**)&platformInstance.chrout = (void*)&m65_chrout;
    *(void**)&platformInstance.readKeyboard = (void*)&m65_readKeyboard;
    *(void**)&platformInstance.keyRepeat = (void*)&m65_keyRepeat;
    *(void**)&platformInstance.clearKeyBuffer = (void*)&m65_clearKeyBuffer;
    *(void**)&platformInstance.isKeyOrJoystickPressed = (void*)&m65_isKeyOrJoystickPressed;
    *(void**)&platformInstance.readJoystick = (void*)&m65_readJoystick;
    *(void**)&platformInstance.loadMap = (void*)&m65_loadMap;
    *(void**)&platformInstance.loadTileset = (void*)&m65_loadTileset;
    *(void**)&platformInstance.displayImage = (void*)&m65_displayImage;
    *(void**)&platformInstance.generateTiles = (void*)&m65_generateTiles;
    *(void**)&platformInstance.updateTiles = (void*)&m65_updateTiles;
    *(void**)&platformInstance.renderTile = (void*)&m65_renderTile;
    *(void**)&platformInstance.renderTiles = (void*)&m65_renderTiles;
    *(void**)&platformInstance.renderItem = (void*)&m65_renderItem;
    *(void**)&platformInstance.renderKey = (void*)&m65_renderKey;
    *(void**)&platformInstance.renderHealth = (void*)&m65_renderHealth;
    *(void**)&platformInstance.renderFace = (void*)&m65_renderFace;
    *(void**)&platformInstance.renderLiveMap = (void*)&m65_renderLiveMap;
    *(void**)&platformInstance.renderLiveMapTile = (void*)&m65_renderLiveMapTile;
    *(void**)&platformInstance.renderLiveMapUnits = (void*)&m65_renderLiveMapUnits;
    *(void**)&platformInstance.showCursor = (void*)&m65_showCursor;
    *(void**)&platformInstance.hideCursor = (void*)&m65_hideCursor;
    *(void**)&platformInstance.setCursorShape = (void*)&m65_setCursorShape;
    *(void**)&platformInstance.copyRect = (void*)&m65_copyRect;
    *(void**)&platformInstance.clearRect = (void*)&m65_clearRect;
    *(void**)&platformInstance.fillRect = (void*)&m65_fillRect;
    *(void**)&platformInstance.startShakeScreen = (void*)&m65_startShakeScreen;
    *(void**)&platformInstance.shakeScreen = (void*)&m65_shakeScreen;
    *(void**)&platformInstance.stopShakeScreen = (void*)&m65_stopShakeScreen;
    *(void**)&platformInstance.startFadeScreen = (void*)&m65_startFadeScreen;
    *(void**)&platformInstance.fadeScreen = (void*)&m65_fadeScreen;
    *(void**)&platformInstance.stopFadeScreen = (void*)&m65_stopFadeScreen;
    *(void**)&platformInstance.writeToScreenMemory = (void*)&m65_writeToScreenMemory;
    *(void**)&platformInstance.writeToScreenMemoryColor = (void*)&m65_writeToScreenMemoryColor;
    *(void**)&platformInstance.playNote = (void*)&m65_playNote;
    *(void**)&platformInstance.stopNote = (void*)&m65_stopNote;
    *(void**)&platformInstance.loadModule = (void*)&m65_loadModule;
    *(void**)&platformInstance.playModule = (void*)&m65_playModule;
    *(void**)&platformInstance.pauseModule = (void*)&m65_pauseModule;
    *(void**)&platformInstance.stopModule = (void*)&m65_stopModule;
    *(void**)&platformInstance.playSample = (void*)&m65_playSample;
    *(void**)&platformInstance.stopSample = (void*)&m65_stopSample;
    *(void**)&platformInstance.renderFrame = (void*)&m65_renderFrame;
    *(void**)&platformInstance.waitForScreenMemoryAccess = (void*)&m65_waitForScreenMemoryAccess;
    *(void**)&platformInstance.setHighlightedMenuRow = (void*)&m65_setHighlightedMenuRow;

    setup_viciv_screen();

    for (i = 0; i < 256; i++) {
        audioxbar_setcoefficient((uint8_t)i, 0xff);
    }
}
