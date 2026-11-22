#!/bin/sh
# Host-only checks. Never add tests/mock to the CodeVisionAVR include path.
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
output_dir=$(mktemp -d "${TMPDIR:-/tmp}/snake-host-validation.XXXXXX")
host_cc=${CC:-cc}

"$host_cc" -std=c89 -Wall -Wextra -Werror -I"$repo_dir/tests/mock" \
    "$repo_dir/tests/mock_display.c" \
    "$repo_dir/tests/host_snake.c" -o "$output_dir/host_snake"
"$output_dir/host_snake"

"$host_cc" -std=c89 -Wall -Wextra -Werror -I"$repo_dir/tests/mock" \
    "$repo_dir/tests/mock_display.c" \
    "$repo_dir/tests/host_dynamic.c" \
    -o "$output_dir/host_dynamic"
"$output_dir/host_dynamic"

"$host_cc" -std=c89 -Wall -Wextra -Werror -I"$repo_dir/tests/mock" \
    "$repo_dir/drivers/lcd1602.c" \
    "$repo_dir/tests/host_lcd.c" -o "$output_dir/host_lcd"
"$output_dir/host_lcd"

"$host_cc" -DHOST_TEST -std=c89 -Wall -Wextra -Werror -I"$repo_dir/tests/mock" \
    "$repo_dir/drivers/led7seg.c" "$repo_dir/tests/mock_avr.c" \
    "$repo_dir/tests/host_led7seg.c" -o "$output_dir/host_led7seg"
"$output_dir/host_led7seg"

echo "Host validation binaries: $output_dir"
