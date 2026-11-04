#pragma once

/* The game's actions on a DualShock 3. Same interface as the Wii port's wii_input, which pb.cpp calls. */
class ps3_input
{
public:
	static void Initialize();
	static void ScanPads();

	static bool Pause();
	static bool NewGame();
	static bool SkipError();

	static bool LaunchBallDown();
	static bool LaunchBallUp();
	static bool MoveLeftPaddleDown();
	static bool MoveLeftPaddleUp();
	static bool MoveRightPaddleDown();
	static bool MoveRightPaddleUp();
	static bool NudgeLeftDown();
	static bool NudgeLeftUp();
	static bool NudgeRightDown();
	static bool NudgeRightUp();
	static bool NudgeUpDown();
	static bool NudgeUpUp();
};
