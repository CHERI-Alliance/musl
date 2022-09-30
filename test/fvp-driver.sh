#!/bin/bash

CMD_ARGS=()
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
			CMD_ARGS+=(${1})
			shift
			;;
	esac
done
TEST=${1}
shift
TEST_ARGS=${@}

# Ensure FVP allocation pipe is defined
if ! [[ -p "${FVP_PIPE}" ]]; then
	echo "Error: FVP_PIPE is not defined" >&2
	exit 3
fi

# Ensure that the FVP shared directory is defined
if [[ -z "${FVP_SHARED_DIR}" ]]; then
	echo "Error: FVP_SHARED_DIR is not defined" >&2
	exit 4
fi

# Create pipe for IPC
TMP=$(mktemp -d)
PIPE=${TMP}/pipe
mkfifo ${PIPE}

# Push pipe onto FVP allocation pipe
echo ${PIPE} > ${FVP_PIPE}

# Read FVP port from new IPC pipe
read FVP_PORT < ${PIPE}

# Trap release of port and deletion of temporary directory
trap "echo ${FVP_PORT} > ${PIPE}; rm -rf ${TMP}" EXIT

# Test invocation command.  This command adds prefixes to the output of the
# test so that output on stdout and stderr can be propagated properly.
TEST_CMD="send -- \"{ { cd ${FVP_SHARED_DIR} &> /dev/null; ${TEST} ${TEST_ARGS}; echo \$?; } 2>&3 | sed 's/^/STDOUT: /'; } 3>&1 1>&2 | sed 's/^/STDERR: /'\r\""

# Generate commands for propagating environment variables from the current
# environment
ENV_CMDS="$(
env --null | while read -d '' -r DEF; do
	if (( ${#DEF} < 100 )); then
		LHS=$(echo ${DEF} | cut -d '=' -f 1)
		RHS=$(echo ${DEF} | cut -d '=' -f 2-)
		echo "send -- \"export ${LHS}=${RHS@Q}\r\""
		echo "expect -re {/ # $}"
	fi
done
)"

# Read stdin and generate commands to propagate this to the test when run in
# the FVP
read -t 0 && read -d '' STDIN
STDIN_CMD="$(echo -n "${STDIN}" | sed 's/^.*$/send -- \"&\\r\"/')"

RESULT=$(
expect <<-EOM
	set timeout -1
	log_user 0
	spawn telnet localhost ${FVP_PORT}
	expect -- "Escape character is '^]'.\r\n$"
	sleep 1
	${ENV_CMDS}
	${TEST_CMD}
	${STDIN_CMD}
	expect -re {(.*)\r\n/ # $}
	send_user "\$expect_out(1,string)\n"
	send -- "\r"
	expect -re {telnet> $}
	send -- "\r"
	expect eof
	wait
EOM
)

# Extract STDOUT lines excluding the last (used for return code)
STDOUT=$(echo "${RESULT}" | sed '/^STDOUT: /!d;s/^STDOUT: //' | head -n -1)

# Extract STDERR lines
STDERR=$(echo "${RESULT}" | sed '/^STDERR: /!d;s/^STDERR: //')

# Extract return code
XRC=$(echo "${RESULT}" | sed '/^STDOUT: /!d' | tail -n 1 | egrep -o "[1-9][0-9]*|0")

# Echo stdout
echo -n "${STDOUT}"

# Echo stderr
echo -n "${STDERR}" >&2

# Exit with XRC
exit ${XRC}
