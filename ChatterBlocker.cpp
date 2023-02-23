#include "libevdev-1.0/libevdev/libevdev-uinput.h" /* For managing devices */
#include <fcntl.h> /* For managing files */
#include <csignal> /* For handeling signals */
#include <unordered_map> /* For storing key press times */
#include <cstring> /* For strlen function which gives a string lenght */
#include <fstream> /* For file stream operations */
#include <sstream> /* For string stream operations */

// Public variables
int fd; // Reperesents file descriptor of device file
struct libevdev *dev; // Reperesents a libevdev device
struct libevdev_uinput *uidev; // Reperesents a libevdev uinput device
std::stringstream xinput_device_id; // Reperesents id of device in xorg-xinput

// Getting elapesd time after last keydown
int GetElapsedTime( struct timeval keydown_time ) {
	struct timeval current_time; // Reperesents current time in nanoseconds
	gettimeofday( &current_time , nullptr );
    long seconds  = current_time.tv_sec  - keydown_time.tv_sec;
    long useconds = current_time.tv_usec - keydown_time.tv_usec;
    long elapesd_ms = ( (seconds) * 1000 + useconds / 1000.0 ) + 0.5;
	return elapesd_ms;
}

// Free the memory
void CleanUp( int signum ) {

	printf("\nCleaning up!\n");

	if ( strlen(xinput_device_id.str().c_str()) > 0 )
		std::system(("xinput enable " + xinput_device_id.str() ).c_str());
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

	// Making sure to clean up before exit
	std::signal( SIGINT , CleanUp );
	std::signal( SIGTERM , CleanUp );
	std::signal( SIGHUP , CleanUp );
	std::atexit( [] {CleanUp(0);} );

	// And crashes
	std::signal( SIGSEGV , CleanUp );
	std::signal( SIGABRT , CleanUp );
	std::signal( SIGILL , CleanUp );
	std::signal( SIGFPE , CleanUp );

	// Show help message if no arguments were provided
	if ( argc < 3 ) {
		printf("Hey! :) to use ChatterBlocker you must include the device file and the chatter threshold in milliseconds like this\n");
		printf("sudo ./ChatterBlocker /dev/input/by-id/usb-Logitech_G513_Carbon_Tactile_0B5238613437-event-kbd 80ms\n");
		return 1;
	}

	// Private variables
	int err; // Reperesents error numbers
	struct input_event ev; // Reperesents dev events
	std::unordered_map<int,struct timeval> keydowns_time; // Reperesents key name & time of dev key down events
	std::unordered_map<int,bool> keys_held; // Reperesents key name & key being held or not
	int chatter_threshold = std::stoi(argv[2]); // Reperesents the delay in which you cant type (counts as a chatter)

	// Open the device file in read only
	fd = open( argv[1] , O_RDONLY );
    if ( fd < 0 ) {
        printf("Failed to open input device file!\nMaybe you forgot to use sudo ? ( psst i need permissions! )\n");
		printf("Example usage : sudo ./ChatterBlocker /dev/input/by-id/usb-Logitech_G513_Carbon_Tactile_0B5238613437-event-kbd 80ms\n");
        return 1;
    }

	// Create a libevdev device from the file descriptor
	err = libevdev_new_from_fd( fd , &dev );
	if ( err < 0 ) {
		printf("Failed to create a new libevdev device!\n");
		return 1;
	}

	// Check if the device has keys
	if ( !libevdev_has_event_type( dev , EV_KEY ) ) {
		printf("The given device does not have any keys!\n");
		return 1;
	}

		printf("The give device isn't a valid keyboard proceeding anyway!\n");

	// Create a libevdev uinput device from a libevdev device ( this also creates a device file like /dev/event19 which we should read from )
	err = libevdev_uinput_create_from_device( dev , LIBEVDEV_UINPUT_OPEN_MANAGED , &uidev );
	if ( err < 0 ) {
		printf("Failed to create a new virtual device!\nMaybe you forgot to use sudo ? ( psst i need permissions! )\n");
		return 1;
	}

	// Show monitoring device for chatters message
	printf( "Monitoring <%s> for chatters and writing to <%s> :)\n\n" , libevdev_get_name(dev) , libevdev_uinput_get_devnode(uidev) );
	printf("If you dont see anything happening maybe you choose the wrong device!\n");
	printf("for example /dev/input/by-id/usb-Logitech_G513_Carbon_Tactile_0B5238613437-if01-event-kbd doesn't work for me instead i have to use ");
	printf("either /dev/input/by-id/usb-Logitech_G513_Carbon_Tactile_0B5238613437-event-kbd or usually /dev/input/event5 keep in mind that these event numbers change alot");
	printf("so its best to use /dev/input/by-id as a source.\n");

	// Getting xinput_device_id using terminal commands and a file
	std::system(( "xinput list --id-only '" + std::string(libevdev_get_name(dev)) + "' > xinput_device_id_chatterblocker" ).c_str());
	xinput_device_id << std::ifstream("xinput_device_id_chatterblocker").rdbuf();
	std::system("rm xinput_device_id_chatterblocker");

	// Changing to new chatter less device using xorg-xinput
	err = std::system(( "xinput disable " + xinput_device_id.str() ).c_str());
	if ( err == 32512 )
		printf("xorg-xinput Is not installed! , if you are using xorg and you want to change to the virtual(chatterless) device automatically install xorg-xinput\n");
	else if ( err != 0 )
		printf("Failed to use the chatterless device , check out the source code\n");
	if ( err != 0 )
		printf("You can proceed manually by using %s as your input device\n" , libevdev_uinput_get_devnode(uidev) );
	else
		printf( "\nSuccessfully started using %s as your input device\n" , libevdev_uinput_get_devnode(uidev) );


	// Main loop , writes every dev event to uidev except the chatter ones
	printf("\nChatterLogs : \n");
	while (true) {
		if ( libevdev_next_event( dev , LIBEVDEV_READ_FLAG_NORMAL , &ev ) == LIBEVDEV_READ_STATUS_SUCCESS ) {

			// If event was a keydown event
			if ( ev.type == EV_KEY ) {
				switch (ev.value) {

					// Check for chatters
					case 1:

						if ( GetElapsedTime(keydowns_time[ev.code]) < chatter_threshold ) {
							printf( "Prevented %s from chattering" , libevdev_event_code_get_name( ev.type , ev.code ) );
							printf(" , chattered after %dms\n" , GetElapsedTime(keydowns_time[ev.code]) );
							keydowns_time[ev.code] = ev.time;
							continue;
						}

						keydowns_time[ev.code] = ev.time;

						break;

					// Unblock keyhold after chatter
					case 2:

						if (!keys_held[ev.code]) {
							libevdev_uinput_write_event( uidev , ev.type , ev.code , 1 );
							libevdev_uinput_write_event( uidev , EV_SYN , SYN_REPORT , 0 );
							keys_held[ev.code] = true;
						}

						break;

					default:

						keys_held[ev.code] = false;

						break;

				}
			}

			// Write dev event to uidev
			libevdev_uinput_write_event( uidev , ev.type , ev.code , ev.value );

		}
	}

	return 0;

}
