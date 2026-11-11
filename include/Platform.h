#define PLATFORM_NAME "mega65"
#ifndef _PLATFORM_H
#define _PLATFORM_H

#ifndef PLATFORM_SCREEN_WIDTH
#define PLATFORM_SCREEN_WIDTH 320
#endif

#ifndef PLATFORM_SCREEN_HEIGHT
#define PLATFORM_SCREEN_HEIGHT 200
#endif

#define SCREEN_WIDTH_IN_CHARACTERS (PLATFORM_SCREEN_WIDTH / 8)
#define SCREEN_HEIGHT_IN_CHARACTERS (PLATFORM_SCREEN_HEIGHT / 8)

#ifndef PLATFORM_MAP_COUNT
#define PLATFORM_MAP_COUNT 14
#endif

#ifndef PLATFORM_INTRO_OPTIONS
#define PLATFORM_INTRO_OPTIONS 4
#endif

#ifndef PLATFORM_MAP_WINDOW_TILES_WIDTH
#define PLATFORM_MAP_WINDOW_TILES_WIDTH 11
#endif

#ifndef PLATFORM_MAP_WINDOW_TILES_HEIGHT
#define PLATFORM_MAP_WINDOW_TILES_HEIGHT 7
#endif

#include <stdint.h>
#include <stdbool.h>




typedef uint32_t address_t;

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ABS(a) ((a) >= 0 ? (a) : -(a))

typedef enum {
    Map01,
    Map02,
    Map03,
    Map04,
    Map05,
    Map06,
    Map07,
    Map08,
    Map09,
    Map10,
    Map11,
    Map12,
    Map13,
    Map14
} PlatformMap;

typedef enum {
    ImageIntro,
    ImageGame,
    ImageGameOver
} PlatformImage;

typedef enum {
    ModuleSoundFX,
    ModuleIntro,
    ModuleWin,
    ModuleLose,
    ModuleInGame1,
    ModuleInGame2,
    ModuleInGame3,
    ModuleInGame4
} PlatformModule;

typedef enum {
    JoystickBitRight,
    JoystickBitLeft,
    JoystickBitDown,
    JoystickBitUp,
    JoystickBitRed,
    JoystickBitBlue,
    JoystickBitGreen,
    JoystickBitYellow,
    JoystickBitPlay,
    JoystickBitReverse,
    JoystickBitForward,
    JoystickBitExtra
} PlatformJoystickBits;

#define PlatformJoystickRight   (1 << JoystickBitRight)
#define PlatformJoystickLeft    (1 << JoystickBitLeft)
#define PlatformJoystickDown    (1 << JoystickBitDown)
#define PlatformJoystickUp      (1 << JoystickBitUp)
#define PlatformJoystickRed     (1 << JoystickBitRed)
#define PlatformJoystickBlue    (1 << JoystickBitBlue)
#define PlatformJoystickGreen   (1 << JoystickBitGreen)
#define PlatformJoystickYellow  (1 << JoystickBitYellow)
#define PlatformJoystickPlay    (1 << JoystickBitPlay)
#define PlatformJoystickReverse (1 << JoystickBitReverse)
#define PlatformJoystickForward (1 << JoystickBitForward)
#define PlatformJoystickExtra   (1 << JoystickBitExtra)

typedef enum {
    ShapeUse,
    ShapeSearch,
    ShapeMove
} PlatformCursorShape;

typedef struct Platform {
    bool quit;
    uint8_t* (*standardControls)(void);
    void (*setInterrupt)(void (*interrupt)(void));
    void (*show)(void);
    int (*framesPerSecond)(void);
    void (*chrout)(uint8_t c);
    uint8_t (*readKeyboard)(void);
    void (*keyRepeat)(void);
    void (*clearKeyBuffer)(void);
    bool (*isKeyOrJoystickPressed)(bool gamepad);
    uint16_t (*readJoystick)(bool gamepad);
    void (*loadMap)(PlatformMap map, uint8_t* destination);
    uint8_t* (*loadTileset)(void);
    void (*displayImage)(PlatformImage image);
    void (*generateTiles)(uint8_t* tileData, uint8_t* tileAttributes);
    void (*updateTiles)(uint8_t* tileData, uint8_t* tiles, uint8_t numTiles);
    void (*renderTile)(uint8_t tile, uint16_t x, uint16_t y, uint8_t variant, bool transparent);
    void (*renderTiles)(uint8_t backgroundTile, uint8_t foregroundTile, uint16_t x, uint16_t y, uint8_t backgroundVariant, uint8_t foregroundVariant);
    void (*renderItem)(uint8_t item, uint16_t x, uint16_t y);
    void (*renderKey)(uint8_t key, uint16_t x, uint16_t y);
    void (*renderHealth)(uint8_t health, uint16_t x, uint16_t y);
    void (*renderFace)(uint8_t face, uint16_t x, uint16_t y);
    void (*renderLiveMap)(uint8_t* map);
    void (*renderLiveMapTile)(uint8_t* map, uint8_t x, uint8_t y);
    void (*renderLiveMapUnits)(uint8_t* map, uint8_t* unitTypes, uint8_t* unitX, uint8_t* unitY, uint8_t playerColor, bool showRobots);
    void (*showCursor)(uint16_t x, uint16_t y);
    void (*hideCursor)(void);
    void (*setCursorShape)(PlatformCursorShape shape);
    void (*copyRect)(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height);
    void (*clearRect)(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    void (*fillRect)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color);
    void (*startShakeScreen)(void);
    void (*shakeScreen)(void);
    void (*stopShakeScreen)(void);
    void (*startFadeScreen)(uint16_t color, uint16_t intensity);
    void (*fadeScreen)(uint16_t intensity, bool immediate);
    void (*stopFadeScreen)(void);
    void (*writeToScreenMemory)(address_t address, uint8_t value);
    void (*writeToScreenMemoryColor)(address_t address, uint8_t value, uint8_t color, uint8_t yOffset);
    void (*playNote)(uint8_t note);
    void (*stopNote)(void);
    void (*loadModule)(PlatformModule module);
    void (*playModule)(PlatformModule module);
    void (*pauseModule)(void);
    void (*stopModule)(void);
    void (*playSample)(uint8_t sample);
    void (*stopSample)(void);
    void (*renderFrame)(bool waitForNextFrame);
    void (*waitForScreenMemoryAccess)(void);
    void (*setHighlightedMenuRow)(uint16_t row);
} Platform;

extern Platform* platform;

#endif
