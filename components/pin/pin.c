#include <stdio.h>
#include "soc/reg_base.h" // DR_REG_GPIO_BASE, DR_REG_IO_MUX_BASE
#include "driver/rtc_io.h" // rtc_gpio_*
#include "pin.h"

// GPIO Matrix Registers
/* GPIO OUT registers (pins 0..31) */
#define GPIO_OUT_REG         (DR_REG_GPIO_BASE + 0x04)
#define GPIO_OUT_W1TS_REG    (DR_REG_GPIO_BASE + 0x08)
#define GPIO_OUT_W1TC_REG    (DR_REG_GPIO_BASE + 0x0C)
/* GPIO OUT1 registers (pins 32..39) */
#define GPIO_OUT1_REG        (DR_REG_GPIO_BASE + 0x10)
#define GPIO_OUT1_W1TS_REG   (DR_REG_GPIO_BASE + 0x14)
#define GPIO_OUT1_W1TC_REG   (DR_REG_GPIO_BASE + 0x18)

/* GPIO IN registers (read input levels) */
#define GPIO_IN_REG          (DR_REG_GPIO_BASE + 0x3C)  // low half (0..31)
#define GPIO_IN1_REG         (DR_REG_GPIO_BASE + 0x40)  // high half (32..39)

/* GPIO ENABLE (output enable) registers and W1TS/W1TC variants */
#define GPIO_ENABLE_REG          (DR_REG_GPIO_BASE + 0x20)
#define GPIO_ENABLE_W1TS_REG     (DR_REG_GPIO_BASE + 0x24)
#define GPIO_ENABLE_W1TC_REG     (DR_REG_GPIO_BASE + 0x28)
#define GPIO_ENABLE1_REG         (DR_REG_GPIO_BASE + 0x2C)
#define GPIO_ENABLE1_W1TS_REG    (DR_REG_GPIO_BASE + 0x30)
#define GPIO_ENABLE1_W1TC_REG    (DR_REG_GPIO_BASE + 0x34)

/* Per-pin configuration registers */
#define GPIO_PIN_REG(n) ((DR_REG_GPIO_BASE + 0x88) + ((n) * 4))
#define GPIO_FUNC_OUT_SEL_CFG_REG(n) ((DR_REG_GPIO_BASE + 0x530) + ((n) * 4))

// GPIO PINn Register Fields
#define PAD_DRV   2 

// IO MUX Registers
#define IO_MUX_REG(n) ((DR_REG_IO_MUX_BASE) + (PIN_MUX_REG_OFFSET[(n)]))

// IO MUX Register Fields
#define FUN_WPD      7
#define FUN_WPU      8
#define FUN_IE       9
#define FUN_DRV_S   10 
#define MCU_SEL_S   12

// Macros for changing bits in a register
#define REG(r) (*(volatile uint32_t *)(r))
#define REG_BITS 32
#define REG_SET_BIT(r,b) (REG(r) |= (1 << (b)))
#define REG_CLR_BIT(r,b) (REG(r) &= ~(1 << (b)))
#define REG_GET_BIT(r,b) ((REG(r) >> (b)) & 1)

// Gives byte offset of IO_MUX Configuration Register
// from base address DR_REG_IO_MUX_BASE
static const uint8_t PIN_MUX_REG_OFFSET[] = {
    0x44, 0x88, 0x40, 0x84, 0x48, 0x6c, 0x60, 0x64, // pin  0- 7
    0x68, 0x54, 0x58, 0x5c, 0x34, 0x38, 0x30, 0x3c, // pin  8-15
    0x4c, 0x50, 0x70, 0x74, 0x78, 0x7c, 0x80, 0x8c, // pin 16-23
    0x90, 0x24, 0x28, 0x2c, 0xFF, 0xFF, 0xFF, 0xFF, // pin 24-31
    0x1c, 0x20, 0x14, 0x18, 0x04, 0x08, 0x0c, 0x10, // pin 32-39
};


// Reset the configuration of a pin to not be an input or an output.
// Pull-up is enabled so the pin does not float.
// Return zero if successful, or non-zero otherwise.
int32_t pin_reset(pin_num_t pin)
{
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		rtc_gpio_deinit(pin);
		rtc_gpio_pullup_en(pin);
		rtc_gpio_pulldown_dis(pin);
	}
	// Reset GPIO_PINn_REG: All fields zero
    REG(GPIO_PIN_REG(pin)) = 0;
	// Reset GPIO_FUNCn_OUT_SEL_CFG_REG: GPIO_FUNCn_OUT_SEL=0x100
    REG(GPIO_FUNC_OUT_SEL_CFG_REG(pin)) = 0x100;
	// Reset IO_MUX_x_REG: MCU_SEL=2, FUN_DRV=2, FUN_WPU=1
    if (PIN_MUX_REG_OFFSET[pin] != 0xFF) { // Check if pin has an IO_MUX register
        uint32_t mux_defaults = (1 << FUN_WPU) | (2 << FUN_DRV_S) | (2 << MCU_SEL_S);
        REG(IO_MUX_REG(pin)) = mux_defaults;
    }

	// Now that the pin is reset, set the output level to zero
	if (pin < 34) {
        return pin_set_level(pin, 0);
    }
    return 0;
}

// Enable or disable a pull-up on the pin.
// Return zero if successful, or non-zero otherwise.
int32_t pin_pullup(pin_num_t pin, bool enable)
{
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		if (enable) return rtc_gpio_pullup_en(pin);
		else return rtc_gpio_pullup_dis(pin);
	}

    // Set or clear the FUN_WPU bit in an IO_MUX register
    if (PIN_MUX_REG_OFFSET[pin] != 0xFF) { // Make sure the pin is valid
        if (enable) {
            REG_CLR_BIT(IO_MUX_REG(pin), FUN_WPD); // Disable pull-down
            REG_SET_BIT(IO_MUX_REG(pin), FUN_WPU);
        } else {
            REG_CLR_BIT(IO_MUX_REG(pin), FUN_WPU);
        }
    } else {
        return -1;
    }

	return 0;
}

// Enable or disable a pull-down on the pin.
// Return zero if successful, or non-zero otherwise.
int32_t pin_pulldown(pin_num_t pin, bool enable)
{
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		if (enable) return rtc_gpio_pulldown_en(pin);
		else return rtc_gpio_pulldown_dis(pin);
	}
	// Set or clear the FUN_WPD bit in an IO_MUX register
    if (PIN_MUX_REG_OFFSET[pin] != 0xFF) { // Make sure the pin is valid
        if (enable) {
            REG_CLR_BIT(IO_MUX_REG(pin), FUN_WPU); // Disable pull-up
            REG_SET_BIT(IO_MUX_REG(pin), FUN_WPD);
        } else {
            REG_CLR_BIT(IO_MUX_REG(pin), FUN_WPD);
        }
    } else {
        return -1;
    }

	return 0;
}

// Enable or disable the pin as an input signal.
// Return zero if successful, or non-zero otherwise.
int32_t pin_input(pin_num_t pin, bool enable)
{
	// Set or clear the FUN_IE bit in an IO_MUX register
	if (PIN_MUX_REG_OFFSET[pin] != 0xFF) { // Make sure the pin is valid
        if (enable) {
            REG_SET_BIT(IO_MUX_REG(pin), FUN_IE);
        } else {
            REG_CLR_BIT(IO_MUX_REG(pin), FUN_IE);
        }
    } else {
        return -1;
    }

	return 0;
}

// Enable or disable the pin as an output signal.
// Return zero if successful, or non-zero otherwise.
int32_t pin_output(pin_num_t pin, bool enable)
{
	// Set or clear the I/O pin bit in the ENABLE or ENABLE1 register
    if (enable) {
        if (pin < 32) {
            REG(GPIO_ENABLE_W1TS_REG) = (1 << pin);
        } else {
            REG(GPIO_ENABLE1_W1TS_REG) = (1 << (pin - 32));
        }
    } else {
        if (pin < 32) {
            REG(GPIO_ENABLE_W1TC_REG) = (1 << pin);
        } else {
            REG(GPIO_ENABLE1_W1TC_REG) = (1 << (pin - 32));
        }
    }

    return 0;
}

// Enable or disable the pin as an open-drain signal.
// Return zero if successful, or non-zero otherwise.
int32_t pin_odrain(pin_num_t pin, bool enable)
{
	// Set or clear the PAD_DRIVER bit in a PIN register
    if (pin >= 34) { // 34-39 are input-only
        return -1;
    }

    if (enable) {
        REG_SET_BIT(GPIO_PIN_REG(pin), PAD_DRV);
    } else {
        REG_CLR_BIT(GPIO_PIN_REG(pin), PAD_DRV);
    }

	return 0;
}

// Sets the output signal level if the pin is configured as an output.
// Return zero if successful, or non-zero otherwise.
int32_t pin_set_level(pin_num_t pin, int32_t level)
{
	// Set or clear the I/O pin bit in the OUT or OUT1 register
    if (pin >= 34) { // 34-39 are input-only
        return -1;
    }

    if (level) { // Corresponds to a HIGH signal
        if (pin < 32) {
            REG(GPIO_OUT_W1TS_REG) = (1 << pin);
        } else {
            REG(GPIO_OUT1_W1TS_REG) = (1 << (pin - 32));
        }
    } else { // Corresponds to a LOW signal
        if (pin < 32) {
            REG(GPIO_OUT_W1TC_REG) = (1 << pin);
        } else {
            REG(GPIO_OUT1_W1TC_REG) = (1 << (pin - 32));
        }
    }
	
    return 0;
}

// Gets the input signal level if the pin is configured as an input.
// Return zero or one if successful, or negative otherwise.
int32_t pin_get_level(pin_num_t pin)
{
    if (rtc_gpio_is_valid_gpio(pin)) { // Handle RTC pins first
        return rtc_gpio_get_level(pin);
    }
    
    // Get the I/O pin bit from the IN or IN1 register
    if (pin < 32) {
        return REG_GET_BIT(GPIO_IN_REG, pin);
    } else if (pin < 40) {
        return REG_GET_BIT(GPIO_IN1_REG, (pin - 32));
    } else {
        return -1; // Invalid pin number
    }
}

// Get the value of the input registers, one pin per bit.
// The two 32-bit input registers are concatenated into a uint64_t.
uint64_t pin_get_in_reg(void)
{
    // Read the IN and IN1 registers, return the concatenated values
    uint64_t high_bits = (uint64_t)REG(GPIO_IN1_REG) << 32;
    uint32_t low_bits = REG(GPIO_IN_REG);
    return high_bits | low_bits;
}

// Get the value of the output registers, one pin per bit.
// The two 32-bit output registers are concatenated into a uint64_t.
uint64_t pin_get_out_reg(void)
{
    // Read the OUT and OUT1 registers, return the concatenated values
    uint64_t high_bits = (uint64_t)REG(GPIO_OUT1_REG) << 32;
    uint32_t low_bits = REG(GPIO_OUT_REG);
    return high_bits | low_bits;
}