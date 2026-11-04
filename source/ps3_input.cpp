#include "ps3_input.h"

#include "pad.h"
#include "test_run.h"

namespace
{
	// Shoulder buttons and triggers both flip, so either grip feels natural.
	constexpr unsigned LeftFlipper = PAD_L1 | PAD_L2;
	constexpr unsigned RightFlipper = PAD_R1 | PAD_R2;
	constexpr unsigned Plunger = PAD_CROSS;
	constexpr unsigned NudgeLeft = PAD_LEFT;
	constexpr unsigned NudgeRight = PAD_RIGHT;
	constexpr unsigned NudgeUp = PAD_UP;

	PadEdges frame{};

	bool pressed(unsigned buttons) { return (frame.pressed & buttons) != 0; }
	bool released(unsigned buttons) { return (frame.released & buttons) != 0; }
}

void ps3_input::Initialize() { pad_open(); }
void ps3_input::ScanPads() { frame = pad_read(test_run_scripted_buttons()); }

bool ps3_input::Pause() { return pressed(PAD_START); }
bool ps3_input::NewGame() { return pressed(PAD_SELECT); }
bool ps3_input::SkipError() { return pressed(PAD_CROSS); }

bool ps3_input::LaunchBallDown() { return pressed(Plunger); }
bool ps3_input::LaunchBallUp() { return released(Plunger); }
bool ps3_input::MoveLeftPaddleDown() { return pressed(LeftFlipper); }
bool ps3_input::MoveLeftPaddleUp() { return released(LeftFlipper); }
bool ps3_input::MoveRightPaddleDown() { return pressed(RightFlipper); }
bool ps3_input::MoveRightPaddleUp() { return released(RightFlipper); }
bool ps3_input::NudgeLeftDown() { return pressed(NudgeLeft); }
bool ps3_input::NudgeLeftUp() { return released(NudgeLeft); }
bool ps3_input::NudgeRightDown() { return pressed(NudgeRight); }
bool ps3_input::NudgeRightUp() { return released(NudgeRight); }
bool ps3_input::NudgeUpDown() { return pressed(NudgeUp); }
bool ps3_input::NudgeUpUp() { return released(NudgeUp); }
