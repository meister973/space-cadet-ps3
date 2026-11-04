# Unit tests for the platform-independent modules. Run through tools/test.ps1.
CFLAGS := -std=gnu99 -Wall -Wextra -Werror -O1 -I../platform
TESTS  := test_wav test_pixels test_layout test_pad_edges test_channels test_automation test_file_names

.PHONY: all
all: $(TESTS)
	@for t in $(TESTS); do ./$$t || exit 1; done
	@echo "all tests passed"

test_wav: test_wav.c ../platform/wav.c
test_pixels: test_pixels.c ../platform/pixels.c
test_layout: test_layout.c ../platform/screen_layout.c
test_pad_edges: test_pad_edges.c ../platform/pad_edges.c
test_channels: test_channels.c ../platform/channels.c
test_automation: test_automation.c ../platform/automation.c
test_file_names: test_file_names.c ../platform/file_names.c

$(TESTS):
	$(CC) $(CFLAGS) $^ -lm -o $@

.PHONY: clean
clean:
	rm -f $(TESTS)
