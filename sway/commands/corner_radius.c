#include <string.h>
#include "sway/commands.h"
#include "sway/config.h"
#include "log.h"

struct cmd_results *cmd_corner_radius(int argc, char **argv) {
	struct cmd_results *error = NULL;
	if ((error = checkarg(argc, "corner_radius", EXPECTED_EQUAL_TO, 1))) {
		return error;
	}

	char *inv;
	int value = strtol(argv[0], &inv, 10);
	if (*inv != '\0' || value < 0 || value > 99) {
		return cmd_results_new(CMD_FAILURE, "Invalid size specified");
	}

	config->corner_radius = value;

	// TODO: rerender windows (see opacity cmd)

	return cmd_results_new(CMD_SUCCESS, NULL);
}
