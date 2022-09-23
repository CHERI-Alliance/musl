#!/bin/bash

MIE_ARGS=()
while true; do
	if (( ${#} == 0 )); then
		echo "Error: invalid driver invocation" >&2
		exit 2
	fi
	case ${1} in
		--)
			shift
			break
			;;
		*)
			MIE_ARGS+=(${1})
			shift
			;;
	esac
done
TEST=${1}
shift
TEST_ARGS=${@}

# Ensure MORELLOIE
if [[ -z "${MORELLOIE}" ]]; then
	echo "Error: MORELLOIE is not defined" >&2
	exit 3
fi

${MORELLOIE} ${TEST_RUNNER_MIE_ARGS} ${MIE_ARGS[*]} -- ${TEST} ${TEST_ARGS}
