//ScriptHook
#include <natives.h>
// #include <types.h> //Already included in globals.h
//Custom
#include <string>
#include "utils\functions.h"
#include "utils\actions.h"
#include "utils\menuEntries.h"
#include "globals.h"
#include "script.h"

#pragma warning(disable : 6001 6054 6285)

namespace /* MenuGlobals */ {
// Declare/Initialise global variables here.

enum SUBMENU {
	// Define submenu (enum) indices here.
	CLOSED,
	MAINMENU,
	SETTINGS,
	// Others:
	SMOKE_AND_DRINK,
	HOBBIES_AND_ACTIVITIES,
	TOOLS_AND_UTILITIES,
	MISCELLANEOUS
};

char str[69];
bool padInputBool = true, bNULL = false;
constexpr TextStyle defaultTextStyle = { FONT_STANDARD, 0.35f, 0.35f, RGBA{255, 255, 255, 255}, DROPSTYLE_NONE, 0.0f, 1.0f, TEXTTYPE_TS_STANDARDMEDIUMHUD };
constexpr RGBA whiteRGB = { 255, 255, 255, 255 };
constexpr RGBA titleBoxCol = { 60, 125, 230, 255 };	// 50, 105, 190, 255
constexpr RGBA backgroundCol = { 20, 20, 20, 125 };
constexpr RGBA titleTextCol = { 255, 255, 255, 255 };
constexpr RGBA optionTextCol = { 255, 255, 255, 255 };
constexpr RGBA optionCountCol = { 255, 255, 255, 255 };
constexpr RGBA selectedTextCol = { 255, 255, 255, 255 };
constexpr RGBA selectionHighlightedCol = { 255, 255, 255, 140 };
constexpr TextFonts fontTitle = FONT_CURSIVE, fontOptions = FONT_STANDARD, fontSelection = FONT_STANDARD;
constexpr float menuX = 0.125f, menuY = 0.05f, menuOptionYMult = 0.035f;
float menuXOffset = 0.0f, menuYOffset = 0.0f, optionCoord = 0.0f, menuYStart = 0.0f;
inline float GetMenuX() { return menuX + menuXOffset; }
inline float GetMenuY() { return menuY + menuYOffset; }
Vehicle playerVeh;

char* menuTitle = "ActionsV";

// Booleans for loops go here:
//bool test = 0;
}	// END MenuGlobals

namespace /* MenuUtils */ {
// Define subroutines here.

void nullFunc() { return; }

void VectorToFloat(Vector3 vec, float Out[])
{
	Out[0] = vec.x;
	Out[1] = vec.y;
	Out[2] = vec.z;
}

void DrawRect(float x, float y, float width, float height, RGBA col = whiteRGB)
{
	DRAW_RECT(x, y, width, height, col.R, col.G, col.B, col.A, false);
}

void DrawSprite(const char* textureDict, const char* textureName, float screenX, float screenY, float width, float height, float heading, RGBA col = whiteRGB)
{
	DRAW_SPRITE(textureDict, textureName, screenX, screenY, width, height, heading, col.R, col.G, col.B, col.A, false, NULL);
}

// relativeWidth: relative width of the sprite  (0.0-1.0, 1.0 means the whole screen width)
float GetSpriteHeight(char* txd, char* tx, float relativeWidth)
{
	const Vector3 txRes = GET_TEXTURE_RESOLUTION(txd, tx);
	const float txResRatio = txRes.x / txRes.y;
	const float height = relativeWidth * GET_SCREEN_ASPECT_RATIO() * txResRatio;
	return height;
}

bool LoadTextureDictionary(char* txd)
{
	if (HAS_STREAMED_TEXTURE_DICT_LOADED(txd))
		return true;
	else
		REQUEST_STREAMED_TEXTURE_DICT(txd, false);

	return false;
}

bool CheckPlayerInsideVehicle()
{
	if (IS_PED_IN_ANY_VEHICLE(PLAYER_PED_ID(), 0))
		return true;

	return false;
}

int FindFreeCarSeat(int vehicle)
{
	int max = GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS(vehicle) - 2;
	for (int tick = -1; tick <= max; tick++)
	{
		if (IS_VEHICLE_SEAT_FREE(vehicle, tick, false))
		{
			return tick;
		}
	}

	return -1;
}

void GetOffsetFromEntity(Hash entity, float X, float Y, float Z, float Out[])
{
	VectorToFloat(GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(entity, X, Y, Z), Out);
}

void SetTextStyle(TextStyle Style = defaultTextStyle, bool bDrawBeforeFade = false)
{
	// choose font
	SET_TEXT_FONT(Style.aFont);
	if (Style.WrapStartX != 0.0f || Style.WrapEndX != 0.0f)
		SET_TEXT_WRAP(Style.WrapStartX, Style.WrapEndX);

	SET_TEXT_SCALE(Style.XScale, Style.YScale);
	SET_TEXT_COLOUR(Style.colour.R, Style.colour.G, Style.colour.B, Style.colour.A);

	switch (Style.drop)
	{
	case DROPSTYLE_NONE:
		break;
	case DROPSTYLE_ALL:
		SET_TEXT_OUTLINE();
		SET_TEXT_DROP_SHADOW();
		break;
	case DROPSTYLE_DROPSHADOWONLY:
		SET_TEXT_DROP_SHADOW();
		break;
	case DROPSTYLE_OUTLINEONLY:
		SET_TEXT_OUTLINE();
		break;
	}

	if (bDrawBeforeFade)
		SET_SCRIPT_GFX_DRAW_ORDER(GFX_ORDER_AFTER_HUD);

	//Other
	SET_TEXT_CENTRE(false);
	SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
	SET_TEXT_EDGE(0, 0, 0, 0, 0);
	return;
}

void DrawString(char* text, float X, float Y)
{
	BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
	ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text);
	END_TEXT_COMMAND_DISPLAY_TEXT(X, Y, 0);
}
void DrawInt(int text, float X, float Y)
{
	BEGIN_TEXT_COMMAND_DISPLAY_TEXT("NUMBER");
	ADD_TEXT_COMPONENT_INTEGER(text);
	END_TEXT_COMMAND_DISPLAY_TEXT(X, Y, 0);
}
void DrawFloat(float text, int decimal_places, float X, float Y)
{
	BEGIN_TEXT_COMMAND_DISPLAY_TEXT("NUMBER");
	ADD_TEXT_COMPONENT_FLOAT(text, decimal_places);
	END_TEXT_COMMAND_DISPLAY_TEXT(X, Y, 0);
}

void PlaySoundFrontend(char* sound_dict, char* sound_name)
{
	PLAY_SOUND_FRONTEND(-1, sound_name, sound_dict, false);
}
void PlayDefaultSoundFrontend(char* sound_name)
{
	PLAY_SOUND_FRONTEND(-1, sound_name, "HUD_FRONTEND_DEFAULT_SOUNDSET", false);
}

bool CheckStringLength(char* str1, size_t max_length)
{
	if (strlen(str1) <= max_length)
		return true;

	return false;
}

char* ConcatenateStrings(char* string1, char* string2)
{
	memset(str, 0, sizeof(str));
	strcpy_s(str, "");
	strcpy_s(str, string1);
	strcat_s(str, string2);
	return str;
}
char* ConcatenateStringInt(char* string1, int int2)
{
	char* Return = "";
	printf_s(Return, "%i", int2);
	Return = ConcatenateStrings(string1, Return);
	return Return;

	/*std::string string2 = string1;
	string2 += std::to_string(int2);

	char * Char = new char[string2.size() + 1];
	std::copy(string2.begin(), string2.end(), Char);
	Char[string2.size()] = '\0';

	char* Return = Char;
	delete[] Char;
	return Return;*/
}

int StringToInt(char* text)
{
	int tempp;
	if (text == "")
		return NULL;

	if (STRING_TO_INT(text, &tempp))
		return NULL;

	return tempp;
}
}	// END MenuUtils

namespace /* Menu */ {
class Menu
{
public:
	static unsigned short currentSubmenu;
	static unsigned short previousSubmenu;
	static unsigned short currentOption;
	static unsigned short totalOptions;
	static unsigned short printingOption;
	static unsigned short breakCount;
	static unsigned short totalBreaks;
	static unsigned char breakScroll;
	static short currentSubArrayIndex;
	static int currentSubArray[20];
	static int currenTopArray[20];
	static int SetSubDelayed;
	static bool bit_centre_title, bit_centre_options;

	static void Loops();
	static void SubmenuHandler();
	static void SubmenuSwitch();

	static void SetupHUD()
	{
		HIDE_HELP_TEXT_THIS_FRAME();
		HIDE_HUD_COMPONENT_THIS_FRAME(HUD_HELP_TEXT);
		HIDE_HUD_COMPONENT_THIS_FRAME(HUD_VEHICLE_NAME);
		HIDE_HUD_COMPONENT_THIS_FRAME(HUD_AREA_NAME);
		HIDE_HUD_COMPONENT_THIS_FRAME(HUD_STREET_NAME);
		HIDE_HUD_COMPONENT_THIS_FRAME(HUD_DISTRICT_NAME);
		//DISPLAY_AMMO_THIS_FRAME(false);
		//DISPLAY_CASH(false);
		SET_RADAR_ZOOM(0);
	}

	static void DisableControls()
	{
		if (!IS_USING_KEYBOARD_AND_MOUSE(NULL))	// Controller
		{
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_HUD_SPECIAL, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_UNARMED, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_MELEE, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_HANDGUN, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_SHOTGUN, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_SMG, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_AUTO_RIFLE, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_SNIPER, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_HEAVY, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_SPECIAL, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_WEAPON_WHEEL_NEXT, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_WEAPON_WHEEL_PREV, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_WEAPON_SPECIAL, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_WEAPON_SPECIAL_TWO, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_DIVE, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MELEE_ATTACK_LIGHT, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MELEE_ATTACK_HEAVY, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MELEE_BLOCK, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_ARREST, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_VEH_HEADLIGHT, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_VEH_RADIO_WHEEL, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_CONTEXT, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_RELOAD, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_VEH_CIN_CAM, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_JUMP, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_VEH_SELECT_NEXT_WEAPON, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_VEH_FLY_SELECT_NEXT_WEAPON, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_CHARACTER_FRANKLIN, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_CHARACTER_MICHAEL, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_CHARACTER_TREVOR, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_CHARACTER_MULTIPLAYER, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_CHARACTER_WHEEL, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_DETONATE, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SPRINT, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_VEH_DUCK, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_VEH_HEADLIGHT, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_VEH_PUSHBIKE_SPRINT, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_VEH_PUSHBIKE_PEDAL, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_FRONTEND_ACCEPT, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_FRONTEND_CANCEL, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_FRONTEND_DOWN, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_FRONTEND_UP, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_FRONTEND_LEFT, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_FRONTEND_RIGHT, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_FRONTEND_RDOWN, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_UP, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_DOWN, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_LEFT, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_RIGHT, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_SELECT, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CANCEL, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_OPTION, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_EXTRA_OPTION, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_SCROLL_FORWARD, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_SCROLL_BACKWARD, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_FOCUS_LOCK, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_GRID, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_SELFIE, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_DOF, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_EXPRESSION, padInputBool);
		}
		else	// Keyboard
		{
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_LOOK_BEHIND, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_NEXT_CAMERA, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_VEH_SELECT_NEXT_WEAPON, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_VEH_CIN_CAM, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_HUD_SPECIAL, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_CHARACTER_FRANKLIN, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_CHARACTER_MICHAEL, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_CHARACTER_TREVOR, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_CHARACTER_MULTIPLAYER, padInputBool);
			DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_CHARACTER_WHEEL, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_FRONTEND_ACCEPT, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_FRONTEND_CANCEL, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_FRONTEND_DOWN, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_FRONTEND_UP, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_FRONTEND_LEFT, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_FRONTEND_RIGHT, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_FRONTEND_RDOWN, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_UP, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_DOWN, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_LEFT, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_RIGHT, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_SELECT, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CANCEL, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_OPTION, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_EXTRA_OPTION, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_SCROLL_FORWARD, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_SCROLL_BACKWARD, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_FOCUS_LOCK, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_GRID, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_SELFIE, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_DOF, padInputBool);
			DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_EXPRESSION, padInputBool);
		}

		SET_CINEMATIC_BUTTON_ACTIVE(true);
		SET_INPUT_EXCLUSIVE(FRONTEND_CONTROL, INPUT_FRONTEND_ACCEPT);
		SET_INPUT_EXCLUSIVE(FRONTEND_CONTROL, INPUT_FRONTEND_CANCEL);
		SET_INPUT_EXCLUSIVE(FRONTEND_CONTROL, INPUT_FRONTEND_DOWN);
		SET_INPUT_EXCLUSIVE(FRONTEND_CONTROL, INPUT_FRONTEND_UP);
		SET_INPUT_EXCLUSIVE(FRONTEND_CONTROL, INPUT_FRONTEND_LEFT);
		SET_INPUT_EXCLUSIVE(FRONTEND_CONTROL, INPUT_FRONTEND_RIGHT);
		return;
	}

	static void Base()
	{
		if (Menu::currentSubmenu != SUBMENU::CLOSED)
		{
			char* txd = "commonmenu";
			if (!HAS_STREAMED_TEXTURE_DICT_LOADED(txd))
				REQUEST_STREAMED_TEXTURE_DICT(txd, false);

			DrawBackground();
			DrawHighlightedOption();
		}
	}

	static void DrawBackground()
	{
		constexpr char* txd = "CommonMenu";
		if (!LoadTextureDictionary(txd))
			return;

		// Calculate last option number to draw rect
		float tmpY = 0.0f;
		if (totalOptions > 14)
			tmpY = 14.0f;
		else
			tmpY = static_cast<float>(totalOptions);

		// Calculate Y Coord
		const float bg_Y = ((tmpY * menuOptionYMult) / 2.0f) + GetMenuY() + 0.059f;
		menuYStart = GetMenuY() + 0.059f;
		const float bg_length = tmpY * menuOptionYMult;

		// Draw titlebox
		DrawSprite(txd, "interaction_bgd", GetMenuX(), GetMenuY() + 0.0175f, 0.20f, 0.0835f, 0.0f);

		// Draw background
		DrawSprite(txd, "gradient_bgd", GetMenuX(), bg_Y, 0.20f, bg_length, 0.0f, RGBA{255, 255, 255, 200});
		//DrawRect(GetMenuX(), bg_Y, 0.20f, bg_length, backgroundCol);

		// Draw scroller indicator rect
		if (totalOptions > 14) tmpY = 14.0f; else tmpY = static_cast<float>(totalOptions);
		float scr_rect_Y = ((tmpY + 1.0f) * menuOptionYMult) + GetMenuY() + 0.0415f;
		DrawRect(GetMenuX(), scr_rect_Y, 0.20f, 0.0345f, backgroundCol);

		// Draw thin line over scroller indicator rect
		if (totalOptions < 14)
			DrawRect(GetMenuX(), static_cast<float>(totalOptions) * menuOptionYMult + GetMenuY() + 0.06f, 0.20f, 0.0022f, whiteRGB);
		else
			DrawRect(GetMenuX(), 14.0f * menuOptionYMult + GetMenuY() + 0.06f, 0.20f, 0.0022f, whiteRGB);

		// Draw scroller indicator
		if ((totalOptions > 14))
		{
			constexpr char* tx1 = "shop_arrows_upanddown";
			constexpr char* tx2 = "arrowright";
			tmpY = ((14.0f + 1.0f) * menuOptionYMult) + GetMenuY() + 0.042375f;

			constexpr float width = 0.025f;
			if (currentOption == 1)
				DrawSprite(txd, tx2, GetMenuX(), tmpY, width, GetSpriteHeight(txd, tx2, width), 270.0f, titleBoxCol);
			else if (currentOption == totalOptions)
				DrawSprite(txd, tx2, GetMenuX(), tmpY, width, GetSpriteHeight(txd, tx2, width), 90.0f, titleBoxCol);
			else
				DrawSprite(txd, tx1, GetMenuX(), tmpY, width, GetSpriteHeight(txd, tx1, width), 0.0f, titleBoxCol);
		}

		// Draw option count
		TextStyle tmpStyle = defaultTextStyle; tmpStyle.aFont = FONT_STANDARD; tmpStyle.XScale = 0.0f; tmpStyle.YScale = 0.26f; tmpStyle.colour = optionCountCol;
		SetTextStyle(tmpStyle);

		BEGIN_TEXT_COMMAND_DISPLAY_TEXT("CM_ITEM_COUNT");
		ADD_TEXT_COMPONENT_INTEGER(currentOption);
		ADD_TEXT_COMPONENT_INTEGER(totalOptions);
		END_TEXT_COMMAND_DISPLAY_TEXT(GetMenuX() + 0.0605f, scr_rect_Y - 0.009f, 0);
	}

	static void DrawHighlightedOption()
	{
		float Y_coord;
		if (currentOption > 14)
			Y_coord = 14.0f;
		else
			Y_coord = static_cast<float>(currentOption);

		Y_coord = (Y_coord * menuOptionYMult) + GetMenuY() + 0.0415f;
		DrawRect(GetMenuX(), Y_coord, 0.20f, 0.035f, selectionHighlightedCol);
	}

	static bool isBinds()
	{
		// Open menu - DPAD Left + LB / F10
		if (IS_USING_KEYBOARD_AND_MOUSE(NULL))
		{
			if (IS_DISABLED_CONTROL_JUST_PRESSED(PLAYER_CONTROL, INPUT_DROP_AMMO))
				return true;
		}
		else
		{
			if ((IS_DISABLED_CONTROL_JUST_PRESSED(FRONTEND_CONTROL, INPUT_FRONTEND_LEFT) && IS_DISABLED_CONTROL_PRESSED(FRONTEND_CONTROL, INPUT_FRONTEND_LB)) ||
				(IS_DISABLED_CONTROL_PRESSED(FRONTEND_CONTROL, INPUT_FRONTEND_LEFT) && IS_DISABLED_CONTROL_JUST_PRESSED(FRONTEND_CONTROL, INPUT_FRONTEND_LB)))
				return true;
		}

		return false;
	}

	static void WhileClosed()
	{
		if (!isBinds())
			return;

		PlaySoundFrontend("FocusIn", "HintCamSounds");
		if (previousSubmenu != SUBMENU::CLOSED)
			currentSubmenu = previousSubmenu;
		else
		{
			currentSubmenu = SUBMENU::MAINMENU;
			currentSubArrayIndex = 0;
			currentOption = 1;
		}
	}
	static void WhileOpened()
	{
		totalOptions = printingOption; printingOption = 0;
		totalBreaks = breakCount; breakCount = 0; breakScroll = 0;

		if (IS_PAUSE_MENU_ACTIVE()) SetSubClosed();
		if (IS_GAMEPLAY_HINT_ACTIVE()) STOP_GAMEPLAY_HINT(false);

		SetupHUD();
		DisableControls();

		// Scroll up
		if (IS_DISABLED_CONTROL_JUST_PRESSED(FRONTEND_CONTROL, INPUT_FRONTEND_UP))
		{
			if (currentOption == 1)
				Bottom();
			else
				Up();
		}

		// Scroll down
		if (IS_DISABLED_CONTROL_JUST_PRESSED(FRONTEND_CONTROL, INPUT_FRONTEND_DOWN))
		{
			if (currentOption == totalOptions)
				Top();
			else
				Down();
		}

		// B press
		if (IS_DISABLED_CONTROL_JUST_PRESSED(FRONTEND_CONTROL, INPUT_FRONTEND_CANCEL) ||
			(IS_DISABLED_CONTROL_JUST_PRESSED(FRONTEND_CONTROL, INPUT_FRONTEND_DELETE) && IS_USING_KEYBOARD_AND_MOUSE(NULL)))
		{
			if (currentSubmenu == SUBMENU::MAINMENU)
				SetSubClosed();
			else
				SetSubPrevious();
		}

		// Binds press
		if (isBinds())
			SetSubClosed();
	}

	static void Up()
	{
		currentOption--;
		PlayDefaultSoundFrontend("NAV_UP_DOWN");
		breakScroll = 1;
	}
	static void Down()
	{
		currentOption++;
		PlayDefaultSoundFrontend("NAV_UP_DOWN");
		breakScroll = 2;
	}
	static void Bottom()
	{
		currentOption = totalOptions;
		PlayDefaultSoundFrontend("NAV_UP_DOWN");
		breakScroll = 3;
	}
	static void Top()
	{
		currentOption = 1;
		PlayDefaultSoundFrontend("NAV_UP_DOWN");
		breakScroll = 4;
	}

	static void SetSubPrevious()
	{
		currentSubmenu = currentSubArray[currentSubArrayIndex]; // Get previous submenu from array and set as current submenu
		currentOption = currenTopArray[currentSubArrayIndex]; // Get last selected option from array and set as current selected option
		currentSubArrayIndex--; // Decrement array index by 1
		printingOption = 0; // Reset option print variable
		PlayDefaultSoundFrontend("BACK"); // Play sound
	}
	static void SetSubNew(int sub_index)
	{
		currentSubArrayIndex++; // Increment array index
		currentSubArray[currentSubArrayIndex] = currentSubmenu; // Store current submenu index in array
		currentSubmenu = sub_index; // Set new submenu as current submenu (Static_1)

		currenTopArray[currentSubArrayIndex] = currentOption; // Store currently selected option in array
		currentOption = 1; // Set new selected option as first option in submenu
		printingOption = 0; // Reset currently printing option var
	}
	static void SetSubClosed()
	{
		ENABLE_ALL_CONTROL_ACTIONS(PLAYER_CONTROL);
		ENABLE_ALL_CONTROL_ACTIONS(FRONTEND_CONTROL);
		PlayDefaultSoundFrontend("BACK");
		currentSubmenu = SUBMENU::CLOSED;
	}
};

unsigned short Menu::currentSubmenu = SUBMENU::CLOSED;
unsigned short Menu::previousSubmenu = SUBMENU::CLOSED;
unsigned short Menu::currentOption = 0;
unsigned short Menu::totalOptions = 0;
unsigned short Menu::printingOption = 0;
unsigned short Menu::breakCount = 0;
unsigned short Menu::totalBreaks = 0;
unsigned char Menu::breakScroll = 0;
short Menu::currentSubArrayIndex = 0;
int Menu::currentSubArray[20] = {};
int Menu::currenTopArray[20] = {};
int Menu::SetSubDelayed = 0;
bool Menu::bit_centre_title = true, Menu::bit_centre_options = false;

bool IsOptionPressed()
{
	if (IS_DISABLED_CONTROL_JUST_PRESSED(FRONTEND_CONTROL, INPUT_FRONTEND_ACCEPT))
	{
		PlayDefaultSoundFrontend("SELECT");
		return true;
	}
	return false;
}
bool IsOptionRightPressed()
{
	if (IS_DISABLED_CONTROL_JUST_PRESSED(FRONTEND_CONTROL, INPUT_FRONTEND_RIGHT))
	{
		PlayDefaultSoundFrontend("NAV_LEFT_RIGHT");
		return true;
	}
	return false;
}
bool IsOptionLeftPressed()
{
	if (IS_DISABLED_CONTROL_JUST_PRESSED(FRONTEND_CONTROL, INPUT_FRONTEND_LEFT))
	{
		PlayDefaultSoundFrontend("NAV_LEFT_RIGHT");
		return true;
	}
	return false;
}

void AddTitle(char* text)
{
	TextStyle tmpStyle = defaultTextStyle; tmpStyle.aFont = fontTitle; tmpStyle.colour = titleTextCol;
	SetTextStyle(tmpStyle);

	if (Menu::bit_centre_title)
	{
		SET_TEXT_CENTRE(true);
		optionCoord = GetMenuX(); // X coord
	}
	else
		optionCoord = GetMenuX() - 0.094f; // X coord

	if (CheckStringLength(text, 14))
	{
		SET_TEXT_SCALE(0.75f, 0.75f);
		DrawString(text, optionCoord, GetMenuY());
	}
	else
		DrawString(text, optionCoord, GetMenuY() + 0.02f);
}

void AddOption(char* text, bool& option_code_bool = bNULL, void(&Func)() = nullFunc, int submenu_index = -1, bool show_arrow = 0)
{
	SET_SCRIPT_VARIABLE_HUD_COLOUR(titleBoxCol.R, titleBoxCol.G, titleBoxCol.B, titleBoxCol.A);
	char* tmpChar = "";
	if (fontOptions == FONT_ROCKSTAR_TAG || fontOptions == FONT_STYLE_PRICEDOWN) tmpChar = "  ------"; // Font unsafe
	else tmpChar = " ~v~>"; // Font safe

	if (Menu::printingOption + 1 == Menu::currentOption && (fontSelection == FONT_ROCKSTAR_TAG || fontSelection == FONT_STYLE_PRICEDOWN)) tmpChar = "  ------"; // Font unsafe
	else tmpChar = " ~v~>"; // Font safe

	Menu::printingOption++;

	optionCoord = 0.0f;
	if (Menu::currentOption <= 14)
	{
		if (Menu::printingOption > 14)
			return;
		else
			optionCoord = (static_cast<float>(Menu::printingOption) * menuOptionYMult) + GetMenuY() + 0.027f;
	}
	else
	{
		if (Menu::printingOption < (Menu::currentOption - 13) || Menu::printingOption > Menu::currentOption)
			return;
		else
			optionCoord = (static_cast<float>(Menu::printingOption - (Menu::currentOption - 14)) * menuOptionYMult) + GetMenuY() + 0.027f;
	}

	TextStyle tmpStyle = defaultTextStyle;
	if (Menu::printingOption == Menu::currentOption)
	{
		tmpStyle.aFont = fontSelection; tmpStyle.colour = selectedTextCol;
		if (IsOptionPressed())
		{
			/*if (&option_code_bool != &null)*/ option_code_bool = true;
			Func();
			if (submenu_index != -1) Menu::SetSubDelayed = submenu_index;
		}
	}
	else
	{
		tmpStyle.aFont = fontOptions; tmpStyle.colour = optionTextCol;
	}

	SetTextStyle(tmpStyle);

	if (show_arrow || submenu_index != -1) text = ConcatenateStrings(text, tmpChar);
	if (Menu::bit_centre_options)
	{
		SET_TEXT_CENTRE(true);
		DrawString(text, GetMenuX(), optionCoord);
	}
	else
		DrawString(text, GetMenuX() - 0.094f, optionCoord);
}

void OptionStatus(int status)
{
	if (optionCoord < menuYStart)
		return;

	char* txd = "CommonMenu";
	if (!LoadTextureDictionary(txd))
		return;

	char* tx1 = "shop_box_blank";
	char* tx2 = "shop_box_tick";
	char* tx3 = "shop_box_cross";
	if (Menu::printingOption == Menu::currentOption)
	{
		tx1 = "shop_box_blankb";
		tx2 = "shop_box_tickb";
		tx3 = "shop_box_crossb";
	}

	constexpr float width = 0.025f, yOffset = 0.0145f;
	switch (status) {
	case 0:
		DrawSprite(txd, tx1, GetMenuX() + 0.073f, optionCoord + yOffset, width, GetSpriteHeight(txd, tx1, width), 0.0f);
		break;
	case 1:
		DrawSprite(txd, tx2, GetMenuX() + 0.073f, optionCoord + yOffset, width, GetSpriteHeight(txd, tx2, width), 0.0f);
		break;
	default:
		DrawSprite(txd, tx3, GetMenuX() + 0.073f, optionCoord + yOffset, width, GetSpriteHeight(txd, tx3, width), 0.0f);
		break;
	}
	return;
}

void AddToggle(char* text, bool& loop_variable, int optionStatus = -1, bool& extra_option_code_ON = bNULL, bool& extra_option_code_OFF = bNULL)
{
	bNULL = NULL;
	AddOption(text, bNULL);

	if (bNULL) {
		loop_variable = !loop_variable;
		if (loop_variable != 0) extra_option_code_ON = true;
		else extra_option_code_OFF = true;
	}

	if (optionStatus <= -1)
		OptionStatus(static_cast<int>(loop_variable)); // Display ON/OFF
	else
		OptionStatus(optionStatus);
}

void AddLocal(char* text, bool condition, bool& option_code_ON, bool& option_code_OFF, int optionStatus = -1)
{
	bNULL = 0;
	AddOption(text, bNULL);
	if (bNULL)
	{
		if (!condition)
			option_code_ON = true;
		else
			option_code_OFF = true;
	}

	if (optionStatus <= -1)
		OptionStatus(condition); // Display ON/OFF
	else
		OptionStatus(optionStatus);
}

void AddNumber(char* text, float value, unsigned char decimal_places, bool& A_PRESS = bNULL, bool& RIGHT_PRESS = bNULL, bool& LEFT_PRESS = bNULL)
{
	bNULL = 0;
	AddOption(text, bNULL);

	if (optionCoord > menuYStart)
	{
		SET_TEXT_FONT(0);
		SET_TEXT_SCALE(0.26f, 0.26f);
		SET_TEXT_CENTRE(1);

		DrawFloat(value, decimal_places, GetMenuX() + 0.073f, optionCoord + 0.004f);
	}

	if (Menu::printingOption == Menu::currentOption)
	{
		if (bNULL)
			A_PRESS = true;
		else if (IsOptionRightPressed())
			RIGHT_PRESS = true;
		else if (IsOptionLeftPressed())
			LEFT_PRESS = true;
	}
}

void AddTeleport(char* text, float X, float Y, float Z, bool& extra_option_code = bNULL)
{
	bNULL = 0;
	AddOption(text, bNULL);
	if (Menu::printingOption == Menu::currentOption)
	{
		if (bNULL)
		{
			if (!CheckPlayerInsideVehicle())
				SET_ENTITY_COORDS(PLAYER_PED_ID(), X, Y, Z, 0, 0, 0, 1);
			else
				SET_ENTITY_COORDS(playerVeh, X, Y, Z, 0, 0, 0, 1);

			LOAD_ALL_OBJECTS_NOW();
			extra_option_code = true;
		}
	}

}
}	// END Menu

namespace Submenus {
// Define submenus here.

void MainMenu()
{
	AddTitle(menuTitle);
	AddOption("Smoke & Drink", bNULL, nullFunc, SUBMENU::SMOKE_AND_DRINK);
	AddOption("Hobbies & Activities", bNULL, nullFunc, SUBMENU::HOBBIES_AND_ACTIVITIES);
	AddOption("Tools & Utilities", bNULL, nullFunc, SUBMENU::TOOLS_AND_UTILITIES);
	AddOption("Miscellaneous", bNULL, nullFunc, SUBMENU::MISCELLANEOUS);
	AddOption("Settings", bNULL, nullFunc, SUBMENU::SETTINGS);
}

void Settings()
{
	bool MenuXOffsetPlus = false, MenuXOffsetMinus = false, MenuYOffsetPlus = false, MenuYOffsetMinus = false;

	AddTitle("Settings");
	AddToggle("Centre Title", Menu::bit_centre_title);
	AddToggle("Centre Options", Menu::bit_centre_options);
	AddNumber("Menu X Offset", menuXOffset, 2, bNULL, MenuXOffsetPlus, MenuXOffsetMinus);
	AddNumber("Menu Y Offset", menuYOffset, 2, bNULL, MenuYOffsetPlus, MenuYOffsetMinus);

	if (MenuXOffsetPlus) {
		if (menuXOffset < 0.74f)
			menuXOffset += 0.01f;
		return;
	}
	else if (MenuXOffsetMinus) {
		if (menuXOffset >= 0.01f)
			menuXOffset -= 0.01f;
		return;
	}

	if (MenuYOffsetPlus) {
		if (menuYOffset < 0.34f)
			menuYOffset += 0.01f;
		return;
	}
	else if (MenuYOffsetMinus) {
		if (menuYOffset >= 0.01f)
			menuYOffset -= 0.01f;
		return;
	}
}

int smokingStatus = -1;
int drinkingStatus = -1;
void SmokeAndDrink()
{	
	AddTitle("Actions");

	bool should = false, shouldNot = false;
	AddLocal("Smoke Cigarette", smokingSequence.IsActive(), should, shouldNot, smokingStatus);
	SequenceOption(should, shouldNot, smokingStatus, smokingSequence);

	should = false; shouldNot = false;
	AddLocal("Drink Beer", drinkingSequence.IsActive(), should, shouldNot, drinkingStatus);
	SequenceOption(should, shouldNot, drinkingStatus, drinkingSequence);
	return;
}

int guitarStatus = -1;
int bongosStatus = -1;
int liftCurlBarStatus = -1;
int fishingStatus = -1;
int jogStatus = -1;
void HobbiesAndActivities()
{
	AddTitle("Activities");

	bool should = false; bool shouldNot = false;
	AddLocal("Play Guitar", guitarSequence.IsActive(), should, shouldNot, guitarStatus);
	SequenceOption(should, shouldNot, guitarStatus, guitarSequence);

	should = false; shouldNot = false;
	AddLocal("Play Bongos", bongosSequence.IsActive(), should, shouldNot, bongosStatus);
	SequenceOption(should, shouldNot, bongosStatus, bongosSequence);

	should = false; shouldNot = false;
	AddLocal("Fish", fishingSequence.IsActive(), should, shouldNot, fishingStatus);
	SequenceOption(should, shouldNot, fishingStatus, fishingSequence);

	should = false; shouldNot = false;
	AddLocal("Lift Weights", liftCurlBarSequence.IsActive(), should, shouldNot, liftCurlBarStatus);
	SequenceOption(should, shouldNot, liftCurlBarStatus, liftCurlBarSequence);

	should = false; shouldNot = false;
	AddLocal("Jog", jogSequence.IsActive(), should, shouldNot, jogStatus);
	SequenceOption(should, shouldNot, jogStatus, jogSequence);
	return;
}

int shineTorchStatus = -1;
int clipboardStatus = -1;
int leafBlowerStatus = -1;
int mopSatus = -1;
int mopWithBucketStatus = -1;
void ToolsAndUtilities()
{
	AddTitle("Utilities");

	bool should = false; bool shouldNot = false;
	AddLocal("Torch", shineTorchSequence.IsActive(), should, shouldNot, shineTorchStatus);
	SequenceOption(should, shouldNot, shineTorchStatus, shineTorchSequence);

	should = false; shouldNot = false;
	AddLocal("Clipboard", clipboardSequence.IsActive(), should, shouldNot, clipboardStatus);
	SequenceOption(should, shouldNot, clipboardStatus, clipboardSequence);

	should = false; shouldNot = false;
	AddLocal("Leaf Blower", leafBlowerSequence.IsActive(), should, shouldNot, leafBlowerStatus);
	SequenceOption(should, shouldNot, leafBlowerStatus, leafBlowerSequence);

	should = false; shouldNot = false;
	AddLocal("Mop", mopSequence.IsActive(), should, shouldNot, mopSatus);
	SequenceOption(should, shouldNot, mopSatus, mopSequence);

	should = false; shouldNot = false;
	AddLocal("Mop with Bucket", mopWithBucketSequence.IsActive(), should, shouldNot, mopWithBucketStatus);
	SequenceOption(should, shouldNot, mopWithBucketStatus, mopWithBucketSequence);
	return;
}

int cameraStatus = -1;
int mobileTextStatus = -1;
int binocularsStatus = -1;
int holdBumSignStatus = -1;
void Miscellaneous()

{
	AddTitle("Miscellaneous");

	bool should = false; bool shouldNot = false;
	AddLocal("Camera", cameraSequence.IsActive(), should, shouldNot, cameraStatus);
	SequenceOption(should, shouldNot, cameraStatus, cameraSequence);

	should = false; shouldNot = false;
	AddLocal("Mobile Text", mobileTextSequence.IsActive(), should, shouldNot, mobileTextStatus);
	SequenceOption(should, shouldNot, mobileTextStatus, mobileTextSequence);

	should = false; shouldNot = false;
	AddLocal("Binoculars", binocularsSequence.IsActive(), should, shouldNot, binocularsStatus);
	SequenceOption(should, shouldNot, binocularsStatus, binocularsSequence);

	should = false; shouldNot = false;
	AddLocal("Beggar Sign", holdBumSignSequence.IsActive(), should, shouldNot, holdBumSignStatus);
	SequenceOption(should, shouldNot, holdBumSignStatus, holdBumSignSequence);
	return;
}
}	//END Submenus

void Menu::Loops()
{
	/*	Make calls to functions that you want looped over here, e.g. ambient lights, whale guns, explosions, checks, flying deers, etc.
		Can also be used for (bool) options that are to be executed from many parts of the script. */

	if (currentSubmenu != SUBMENU::CLOSED)
		previousSubmenu = currentSubmenu;

	playerVeh = GET_VEHICLE_PED_IS_IN(PLAYER_PED_ID(), 0); // Store current vehicle
}

void Menu::SubmenuSwitch()
{ // Make calls to submenus over here.

	switch (currentSubmenu)
	{
	case SUBMENU::MAINMENU:					Submenus::MainMenu(); break;
	case SUBMENU::SETTINGS:					Submenus::Settings(); break;
	case SUBMENU::SMOKE_AND_DRINK:			Submenus::SmokeAndDrink(); break;
	case SUBMENU::HOBBIES_AND_ACTIVITIES:	Submenus::HobbiesAndActivities(); break;
	case SUBMENU::TOOLS_AND_UTILITIES:		Submenus::ToolsAndUtilities(); break;
	case SUBMENU::MISCELLANEOUS:			Submenus::Miscellaneous(); break;
	//	case SUB::NAME						sub::name(); break;
	}
}

void Menu::SubmenuHandler()
{
	if (currentSubmenu == SUBMENU::CLOSED)
		WhileClosed();
	else
	{
		SubmenuSwitch();
		if (SetSubDelayed != NULL)
		{
			SetSubNew(SetSubDelayed);
			SetSubDelayed = NULL;
		}

		WhileOpened();
	}
}

void UpdateMenu()
{
	Menu::Base();
	Menu::Loops();
	Menu::SubmenuHandler();
}