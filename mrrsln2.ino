/*
	mrrsln2 - Railroad Signals for LocoNet (2-aspect signaling)
	David Andrs, 2016
*/

// baud rate Loconet communicates on
#define BAUD_RATE               57600
// number of aspects on a signal. has to match the value set on SE8c
#define NUM_ASPECTS             4

// sensor state
#define SENSOR_HI               1
#define SENSOR_LO               0
// switch state
#define SWITCH_CLOSED           2
#define SWITCH_THROWN           3
// signal aspects (yellow and flashing yellow are not really used in in this setup)
#define SIGNAL_GREEN            4
#define SIGNAL_RED              5
#define SIGNAL_YELLOW           6
#define SIGNAL_FLASHING_YELLOW  7

// number of blocks on the layout
#define NUM_OF_BLOCKS           7
// number of signals operated by a block
#define SIGNALS_PER_BLOCK       2

// Opcode byte
int opcode = 0;
// Message bytes
char data[128];
// Checksum byte
int chksum = 0;

// Indexing: block ID, i-th signal => signal head ID
const int signals[NUM_OF_BLOCKS][SIGNALS_PER_BLOCK] = {
	{ 3, -1 },
	{ 4, 16 },
	{ 15, -1 },
	{ 6, 8 },
	{ 5, 10 },
	{ 9, -1 },
	{ 5, -1 }
};


int compute_checksum(char msg[], int len) {
	chksum = 0;
	for (int i = 0; i < len; i++)
		chksum = chksum ^ (msg[i] ^ 0xFF);
	return chksum;
}

/**
 * Send the LocoNet message to set the switch state
 *
 * @param id The ID of the switch (1-based)
 * @param state The state to set (SWITCH_CLOSED | SWITCH_THROWN)
 */
void set_switch_state(int id, int state) {
	// IDs on LocoNet are 0-based, but people work with 1-based numbers
	int lnid = id - 1;
	// LocoNet message to send (we are not going to send anything longer than 4 bytes)
	char msg[4];

	msg[0] = 0xB0;
	msg[1] = lnid & 0xFF;
	msg[2] = (lnid & 0xF00) >> 7;
	if (state == SWITCH_CLOSED)
		msg[2] = msg[2] | 0x30;
	msg[3] = compute_checksum(msg, 3);
	Serial.write(msg, 4);

	msg[2] = (lnid & 0xF00) >> 7;
	if (state == SWITCH_CLOSED)
		msg[2] = msg[2] | 0x20;
	msg[3] = compute_checksum(msg, 3);
	Serial.write(msg, 4);
}

/**
 * Set signal aspect
 *
 * @param head_id The head ID on SE8c (zero based)
 * @param aspect The aspect to set (SIGNAL_GREEN | SIGNAL_RED | SIGNAL_YELLOW | SIGNAL_FLASHING_YELLOW)
 */
void set_aspect(int head_id, int aspect) {
	// signal ID
	int sid = 257 + ((head_id - 1) * NUM_ASPECTS / 2);
	// Switch states for setting the signal aspect
	int state1 = 0, state2 = 0;
	switch (aspect) {
		case SIGNAL_GREEN:
			state1 = SWITCH_CLOSED;
			state2 = SWITCH_CLOSED;
			break;

		case SIGNAL_RED:
			state1 = SWITCH_THROWN;
			state2 = SWITCH_CLOSED;
			break;

		case SIGNAL_YELLOW:
			state1 = SWITCH_CLOSED;
			state2 = SWITCH_THROWN;
			break;

		case SIGNAL_FLASHING_YELLOW:
			state1 = SWITCH_THROWN;
			state2 = SWITCH_THROWN;
			break;

		default:
			state1 = 0;
			state2 = 0;
			break;
	}

	if ((state1 != 0) && (state2 != 0)) {
		set_switch_state(sid + 0, state1);
		set_switch_state(sid + 1, state2);
	}
}

/**
 * When sensor changes its state, this function is called
 *
 * @param id The ID of the sensor
 * @param state Either SENSOR_LO or SENSOR_HI
 */
void on_sensor_change(int id, int state) {
	if ((id < 0) || (id >= NUM_OF_BLOCKS))
		return;

	for (int i = 0; i < SIGNALS_PER_BLOCK; i++) {
		// negative number means do not do anything
		if (signals[id][i] >= 0) {
			if (state == SENSOR_LO)
				set_aspect(signals[id][i], SIGNAL_GREEN);
			else
				set_aspect(signals[id][i], SIGNAL_RED);
		}
	}
}

/**
 * Decode the message
 */
void decode_message() {
	if (opcode == 0x83) {
		// power on
	}
	else if (opcode == 0x82) {
		// power off
	}
	else if (opcode == 0xB0) {
		// switch state
	}
	else if (opcode == 0xB2) {
		// sensor state
		int id = ((((data[1] & 0xF) << 8) | data[0]) << 1) | ((data[1] >> 5) & 0x1);
		if (((data[1] >> 4) & 0x1) == 0x1)
			on_sensor_change(id, SENSOR_HI);
		else
			on_sensor_change(id, SENSOR_LO);
	}
	else {
		// we do not care about the message
	}
}

/**
 * Reads the message from LocoNet
 *
 * Fills in opcode, chksum and data
 */
void read_message() {
	opcode = Serial.read();
	int n = 0;                      // The length of the message data
	switch (opcode >> 5) {
		case 0x4: n = 0; break;
		case 0x5: n = 2; break;
		case 0x6: n = 4; break;
		case 0x7: n = Serial.read(); break;
		// this should not happen unless somebody is trying to hack the LocoNet
		default: n = -1; break;
	}

	if (n > 0)
		Serial.readBytes(data, n);

	if (n >= 0)
		chksum = Serial.read();
}

void setup() {
	Serial.begin(BAUD_RATE);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB
	}
}

void loop() {
	if (Serial.available() > 0) {
		read_message();
		decode_message();
	}
	delay(100);
}
