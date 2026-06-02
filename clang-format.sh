#/usr/bin/env bash
#
if [ -z $(which clang-format) ]; then
  echo "* clang-format is not installed!"
  exit 1
fi
#
cd $(dirname ${0})
#
for _file in $(find ./src -type f -iname '*.cpp' -o -iname '*.hpp' -o -iname '*.c' -o -iname '*.h'); do
  echo "- formatting ${_file}"
  clang-format -i ${_file}
done
#
