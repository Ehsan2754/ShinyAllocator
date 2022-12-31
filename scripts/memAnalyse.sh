#!/bin/bash
print_help () {
  echo "Usage: memory_analyse.sh <target>"
  echo "Single target example: memory_analyse.sh <target>"
  echo "Multiple targets example: memory_analyse.sh <target1>,<target2>"
  exit -1
}

TARGETS=$1
if [ -z "$TARGETS" ]; then
   print_help
   exit -1
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
REPO_DIR="$(dirname "$SCRIPT_DIR")"

FLASH_SIZE_BYTES="$((64 * 1024))" # 64 Kbytes for most of the MCU (there are exceptions)
RAM_SIZE_BYTES="$((20 * 1024))"   # 20 Kbytes for all MCU

analyse_target_and_print_result () {
    elf_dir=$target
    if [ -f "$elf_dir" ]; then
        size_str=$(size $elf_dir)
        size_arr=( $size_str )
        text_size_usage=${size_arr[6]}
        data_size_usage=${size_arr[7]}
        bss_size_usage=${size_arr[8]}
        flash_size_usage=$text_size_usage
        ram_size_usage="$(($data_size_usage + $bss_size_usage))"
        flash_usage_pct=$((100 * $flash_size_usage / $FLASH_SIZE_BYTES))
        ram_usage_pct=$((100 * $ram_size_usage / $RAM_SIZE_BYTES))

        if (( ram_usage_pct >= 99 )); then
            status=":red_circle:"
        elif (( flash_usage_pct > 90 )); then
            status=":red_circle:"
        elif (( ram_usage_pct > 90 )); then
            status=":warning:"
        elif (( flash_usage_pct > 90 )); then
            status=":warning:"
        else
            status="ok"
        fi

        echo -n "| $status "
        echo -n "| $target "
        echo -n "| ${flash_size_usage} bytes ($flash_usage_pct%) "
        echo    "| ${ram_size_usage} bytes ($ram_usage_pct%) |"
    fi
}

echo "| Status | Target | Flash usage | RAM usage |"
echo "| ------ | ------ | ----------- | --------- |"

for target in ${TARGETS//,/ }; do
    analyse_target_and_print_result
done