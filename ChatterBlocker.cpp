#include <libevdev-1.0/libevdev/libevdev-uinput.h> /* For managing devices */
#include <fcntl.h> /* For managing files */
#include <csignal> /* For handeling signals */
#include <unordered_map> /* For storing each key press info */
#include <sstream> /* For printf ( format strings ) */

// Public variables
int fd; // Reperesents file descriptor of device file
struct libevdev *dev; // Reperesents a libevdev device
struct libevdev_uinput *uidev; // Reperesents a libevdev uinput device
bool gamemode = false; // Reperesents status of gamemode

// Getting elapesd time from old_time until new_time
int GetElapsedTime( struct timeval old_time , struct timeval new_time ) {
	long seconds  = new_time.tv_sec  - old_time.tv_sec;
	long useconds = new_time.tv_usec - old_time.tv_usec;
	long elapesd_ms = ( ( seconds * 1000 ) + ( useconds / 1000 ) );
	return elapesd_ms;
}

// Toggle gamemode on or off
void ToggleGameMode ( int signum ) {
	gamemode = !gamemode;
	printf("Gamemode : %s\n" , gamemode ? "On" : "Off" );
}

// Free the memory
void CleanUp( int signum ) {

	printf("\nCleaning up!\n");

	libevdev_grab( dev , LIBEVDEV_UNGRAB );
	if (uidev)
		libevdev_uinput_destroy(uidev);
	if (dev)
		libevdev_free(dev);
	if (fd)
		close(fd);

	printf("Exiting!\n");
	_exit(0);

}

// Entry
int main( int argc , char *argv[] ) {

	// Making sure to clean up before exits
	std::signal( SIGINT , CleanUp );
	std::signal( SIGTERM , CleanUp );
	std::signal( SIGHUP , CleanUp );
	std::atexit( [] {CleanUp(0);} );

	// And crashes
	std::signal( SIGSEGV , CleanUp );
	std::signal( SIGABRT , CleanUp );
	std::signal( SIGILL , CleanUp );
	std::signal( SIGFPE , CleanUp );

	// Toggle gamemode on signal SIGUSR1
	std::signal( SIGUSR1 , ToggleGameMode );

	// Show help message if no arguments were provided
	if ( argc < 3 ) {
		printf("Hey! :) to use ChatterBlocker you must include the device file and the chatter threshold in milliseconds like this\n");
		printf("sudo ./ChatterBlocker /dev/input/by-id/usb-Logitech_G513_Carbon_Tactile_0B5238613437-event-kbd 80ms\n");
		return 1;
	}

	// Private variables
	int err; // Reperesents error numbers
	struct input_event ev; // Reperesents dev events
	std::unordered_map<int,struct timeval> last_keyup_times; // Reperesents key name & time of dev key up events
	std::unordered_map<int,bool> keys_held; // Reperesents key name & key being held or not
	int chatter_threshold = std::stoi(argv[2]); // Reperesents the delay after a keyup event in which you cant type

	// Open the device file in read only
	fd = open( argv[1] , O_RDONLY );
	if (!fd) {
		printf("Failed to open input device file!\nMaybe you forgot to use sudo ? ( psst i need permissions! )\n");
		printf("Example usage : sudo ./ChatterBlocker /dev/input/by-id/usb-Logitech_G513_Carbon_Tactile_0B5238613437-event-kbd 80ms\n");
        return 1;
	}

	// Create a libevdev device from the file descriptor
	err = libevdev_new_from_fd( fd , &dev );
	if (err) {
		printf("Failed to create a new libevdev device!\n");
		return 1;
	}

	// Check to see if the device has keys
	if (!libevdev_has_event_type( dev , EV_KEY )) {
		printf("The given device does not have any keys!\n");
		return 1;
	}

	// Check to see if the device is a valid keyboard
	if (!libevdev_has_event_code( dev , EV_KEY , KEY_A )) {
		printf("The give device isn't a valid keyboard proceeding anyway!\n");
	}

	// Create a libevdev uinput device from a libevdev device ( this also creates a device file like /dev/event19 which we should read from )
	err = libevdev_uinput_create_from_device( dev , LIBEVDEV_UINPUT_OPEN_MANAGED , &uidev );
	if (err) {
		printf("Failed to create a new virtual device!\nMaybe you forgot to use sudo ? ( psst i need permissions! )\n");
		return 1;
	}

	// Show monitoring device for chatters message and some hints
	printf( "Monitoring <%s> for chatters and writing to <%s> :)\n\n" , libevdev_get_name(dev) , libevdev_uinput_get_devnode(uidev) );
	printf("If you dont see anything happening maybe you choose the wrong device!\n");
	printf("Check out https://github.com/NotMurPh/ChatterBlocker-Linux#how-do-i-install-and-use- to learn more.\n");

	// Grab the device if already not grabbed so only this program can use the device file and thus makes the xorg use the virtual device
	libevdev_next_event( dev , LIBEVDEV_READ_FLAG_BLOCKING , &ev );
	err = libevdev_grab( dev , LIBEVDEV_GRAB );
	if (err)
		printf("Faild to use the virtual device! , you can proceed manually by disabling your main device using xinput or by using %s as your input device.\n" , libevdev_uinput_get_devnode(uidev) );
	else
		printf( "\nSuccessfully started using %s as your input device.\n" , libevdev_uinput_get_devnode(uidev) );

	// Main loop , writes every dev event to uidev except the chatter ones
	printf("\nChatterLogs : \n");
	while (true) {
		if ( libevdev_next_event( dev , LIBEVDEV_READ_FLAG_BLOCKING , &ev ) == LIBEVDEV_READ_STATUS_SUCCESS ) {

			// If event was a key event
			if ( ev.type == EV_KEY ) {
				switch (ev.value) {

					// On key down
					case 1:

						// Block chatters
						if ( GetElapsedTime( last_keyup_times[ev.code] , ev.time ) < chatter_threshold ) {

							// Dont block chatters if its the first keypress for that key
							if ( last_keyup_times[ev.code].tv_sec == 0 ) {
								libevdev_uinput_write_event( uidev , ev.type , ev.code , ev.value );
								continue;
							}

							// Dont block chatters for W A S D keys if gamemode is enabled 
							if (gamemode) {
								if ( ev.code == 17 || ev.code == 30 || ev.code == 31 || ev.code == 32 ) {
									libevdev_uinput_write_event( uidev , ev.type , ev.code , ev.value );
									continue;
								}
							}

							printf( "Prevented %s from chattering" , libevdev_event_code_get_name( ev.type , ev.code ) );
							printf( " , chattered after %dms.\n" , GetElapsedTime( last_keyup_times[ev.code] , ev.time ) );
							keys_held[ev.code] = true;
							continue;
						}

					break;

					// On key hold
					case 2:

						// If the key is held after a chatter make sure to initiate it first
						if (keys_held[ev.code]) {
							libevdev_uinput_write_event( uidev , ev.type , ev.code , 1 );
							libevdev_uinput_write_event( uidev , EV_SYN , SYN_REPORT , 0 );
							keys_held[ev.code] = false;
						}

					break;

					// On key up
					default:

						keys_held[ev.code] = false;
						last_keyup_times[ev.code] = ev.time;

					break;

				}
			}

			// Write dev event to uidev
			libevdev_uinput_write_event( uidev , ev.type , ev.code , ev.value );

		}
	}

	return 0;

}
