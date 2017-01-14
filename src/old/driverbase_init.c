
/*static ktuxKeyboardDriver keyboardDriver;
static ktuxStdDriverBase keyboardDriverBase = {
	false,						// Driver initialized (always default to false)
	true,						// Driver implements IRQ?
	KeyboardInitialize,			// InitDriver() function pointer
	KeyboardIRQHandler			// HandleIRQ() function pointer
};

int KeyboardDriverInitialize(void)
{
	// Our basic driver init function
	keyboardDriver.driverBase = &keyboardDriverBase;

	// Register driver with kernel
	ktuxRegisterDriver(KEYBOARD_DRIVER, &keyboardDriver);

	// Driver init flag = true
	keyboardDriver.driverBase->driverInitialized = true;

	return 0;
}*/
