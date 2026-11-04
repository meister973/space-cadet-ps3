#include "pch.h"
#include "winmain.h"

#include <cstdarg>
#include <unistd.h>

#include "control.h"
#include "midi.h"
#include "options.h"
#include "pb.h"
#include "pinball.h"
#include "ps3_input.h"
#include "render.h"
#include "Sound.h"
#include "screen_layout.h"
#include "system.h"
#include "test_run.h"
#include "video.h"

namespace
{
	// The table is the left 375 columns of the game's screen, the score panel the rest.
	constexpr int TableWidth = 375;
	constexpr float FrameMilliseconds = 1000.0f / 60.0f;
	constexpr unsigned FatalErrorSeconds = 10;

	void draw_screen()
	{
		auto screen = render::vscreen;
		auto fit = layout_fit(video_width(), video_height(), screen->Width, screen->Height);
		auto scale = fit.width / screen->Width;
		auto tableU = static_cast<float>(TableWidth) / screen->Width;
		auto tableWidth = TableWidth * scale;

		// The table shakes when nudged; the score panel stays put.
		VideoQuad quads[2] = {
			{fit.x + render::get_offset_x() * scale, fit.y + render::get_offset_y() * scale,
			 tableWidth, fit.height, 0.0f, 0.0f, tableU, 1.0f},
			{fit.x + tableWidth, fit.y, fit.width - tableWidth, fit.height, tableU, 0.0f, 1.0f, 1.0f},
		};

		video_upload_screen(reinterpret_cast<const unsigned char*>(screen->BmpBufPtr1), screen->Stride * 4);
		video_draw(quads, 2);
	}

	void dump_frame_if_wanted()
	{
		if (!test_run_wants_dump())
			return;

		char path[64];
		snprintf(path, sizeof path, "/dev_hdd0/tmp/frame_%ld.ppm", test_run_frame());
		video_dump_frame(path);
		system_log("dump %s\n", path);
	}
}

int winmain::bQuit = 0;
int winmain::activated;
int winmain::DispFrameRate = 0;
int winmain::DispGRhistory = 0;
int winmain::single_step = 0;
int winmain::last_mouse_x;
int winmain::last_mouse_y;
int winmain::mouse_down;
int winmain::no_time_loss;

std::string winmain::DatFileName;
bool winmain::ShowSpriteViewer = false;
bool winmain::LaunchBallEnabled = true;
bool winmain::HighScoresEnabled = true;
bool winmain::DemoActive = false;
char *winmain::BasePath;
std::string winmain::FpsDetails;
double winmain::UpdateToFrameRatio;
winmain::DurationMs winmain::TargetFrameTime;
optionsStruct &winmain::Options = options::Options;

int winmain::WinMain(LPCSTR lpCmdLine)
{
	(void)lpCmdLine;
	std::set_new_handler(memalloc_failure);

	system_open();
	system_log("Space Cadet PS3: starting\n");
	if (video_open() != 0)
		PrintFatalError("Could not start video output.\n");
	ps3_input::Initialize();

	BasePath = const_cast<char*>(SYSTEM_DATA_FOLDER);
	test_run_open(BasePath);

	pinball::quickFlag = 0;
	DatFileName = options::get_string("Pinball Data", pinball::get_rc_string(168, 0));

	// The Full Tilt! table is used when its data is present.
	auto cadetDat = fopen(pinball::make_path_name("CADET.DAT").c_str(), "r");
	if (cadetDat)
	{
		fclose(cadetDat);
		DatFileName = "CADET.DAT";
		pb::FullTiltMode = true;
	}

	options::init();
	if (!Sound::Init(Options.SoundChannels, Options.Sounds))
		Options.Sounds = false;
	if (!midi::music_init())
		Options.Music = false;
	if (pb::init())
		PrintFatalError("Could not load game data: %s%s is missing.\n", BasePath, DatFileName.c_str());

	if (video_create_screen(render::vscreen->Width, render::vscreen->Height) != 0)
		PrintFatalError("Could not create the screen texture.\n");
	system_log("screen %dx%d on %dx%d, sounds %d, music %d\n", render::vscreen->Width,
	           render::vscreen->Height, video_width(), video_height(), Options.Sounds, Options.Music);

	pb::reset_table();
	pb::firsttime_setup();
	pb::replay_level(0);

	bQuit = false;
	while (!bQuit && !system_exit_requested())
	{
		test_run_next_frame();
		ps3_input::ScanPads();

		if (ps3_input::Pause())
			pause();
		if (ps3_input::NewGame())
			new_game();

		pb::keydown();
		pb::keyup();

		if (!single_step)
			pb::frame(FrameMilliseconds);

		draw_screen();
		video_present();
		dump_frame_if_wanted();

		if (test_run_frame() % 600 == 0)
			system_log("frame=%ld underruns=%u\n", test_run_frame(), audio_underruns());
	}

	system_log("Space Cadet PS3: exiting\n");
	end_pause();
	Sound::Close();
	options::uninit();
	midi::music_shutdown();
	pb::uninit();
	video_close();
	return 0;
}

void winmain::memalloc_failure()
{
	midi::music_stop();
	Sound::Close();
	char *caption = pinball::get_rc_string(170, 0);
	char *text = pinball::get_rc_string(179, 0);

	PrintFatalError("%s %s\n", caption, text);
}

void winmain::end_pause()
{
	if (single_step)
	{
		pb::pause_continue();
		no_time_loss = 1;
	}
}

void winmain::new_game()
{
	end_pause();
	pb::replay_level(0);
}

void winmain::pause()
{
	pb::pause_continue();
	no_time_loss = 1;
}

void winmain::UpdateFrameRate()
{
	// UPS >= FPS
	auto fps = Options.FramesPerSecond, ups = Options.UpdatesPerSecond;
	UpdateToFrameRatio = static_cast<double>(ups) / fps;
	TargetFrameTime = DurationMs(1000.0 / ups);
}

// There is no text output on screen, so the message goes to the log, and the game returns to the XMB.
void winmain::PrintFatalError(const char *message, ...)
{
	char text[512];
	va_list args;
	va_start(args, message);
	vsnprintf(text, sizeof text, message, args);
	va_end(args);

	system_log("FATAL: %s", text);
	system_log("The full log is in %s\n", SYSTEM_LOG_PATH);
	sleep(FatalErrorSeconds);
	exit(EXIT_FAILURE);
}
