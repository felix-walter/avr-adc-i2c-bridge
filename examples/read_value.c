#include <stdint.h> // int*_t types
#include <stdio.h> // perror
#include <stdlib.h> // exit
#include <math.h> // exit

#include <unistd.h> // open, read, write, sleep
#include <sys/ioctl.h> // ioctl
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> // open
#include <linux/i2c-dev.h> // I2C_SLAVE

#define I2C_NODE "/dev/i2c-1"
#define I2C_ADDR 0x42

#define REF_VOLTAGE 5.0

static int read_register(const int file, const uint8_t num)
{
	uint8_t buf[1];

	buf[0] = num;
	if (write(file, buf, 1) < 0) {
		perror("Failed to write to I2C bus");
		return -1;
	}
	if (read(file, buf, 1) != 1) {
		perror("Failed to read from I2C bus");
		return -1;
	}

	return buf[0];
}

static float read_voltage(const int file)
{
	const int xff = read_register(file, 0);
	const int msb = read_register(file, 1);
	const int lsb = read_register(file, 2);
	const int chk = read_register(file, 3);
	const int xaa = read_register(file, 4);

	if (xff < 0 || msb < 0 || lsb < 0 || chk < 0 || xaa < 0)
		return NAN;

	if (xff != 0xff) {
		fprintf(stderr, "Received wrong initiator: %02x", xff);
		return NAN;
	}
	if (chk != (msb ^ lsb)) {
		fprintf(stderr, "Received wrong checksum: 0x%02x != 0x%02x",
			chk, msb ^ lsb);
		return NAN;
	}
	if (xaa != 0xaa) {
		fprintf(stderr, "Received wrong terminator: %02x", xaa);
		return NAN;
	}

	const int voltage_reading_u16 = (msb << 8) | lsb;

	return (float)voltage_reading_u16 * REF_VOLTAGE / 1024.0;
}

int main(void)
{
	int file;

	if ((file = open(I2C_NODE, O_RDWR)) < 0) {
		perror("Failed to open i2c device");
		exit(EXIT_FAILURE);
	}

	if (ioctl(file, I2C_SLAVE, I2C_ADDR) < 0) {
		perror("Failed to set I2C address");
		exit(EXIT_FAILURE);
	}

	for (;;) {
		const float voltage = read_voltage(file);

		printf("V = %f\n", voltage);

		sleep(1);
	}
}
