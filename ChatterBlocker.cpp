#include <libevdev-1.0/libevdev/libevdev-uinput.h> /* For managing devices */
#include <fcntl.h> /* For managing files */
#include <csignal> /* For handeling signals */
#include <thread> /* For chatter waiters ( Threads ) */
#include <unordered_map> /* For storing each key press info ( Dictionary ) */
#include <sstream> /* For printf ( Format strings ) */
#include <vector> /* For storing arguments ( List ) */

using namespace std;

// Public variables
int fd; // Reperesents file descriptor of device file
libevdev *dev; // Reperesents a libevdev device
libevdev_uinput *uidev; // Reperesents a libevdev uinput device
bool gamemode = false; // Reperesents status of gamemode
bool delaymode = false; // Reperesents status of delaymode

// Shows the help message
void ShowHelp () {

	printf( "Hey! :) to use ChatterBlocker you must include the device file address and the chatter threshold in milliseconds like this\n" );
	printf( "sudo ./ChatterBlocker /dev/input/by-id/usb-Logitech_G513_Carbon_Tactile_0B5238613437-event-kbd 30ms\n" );
	printf( "\nAvalibale optional flags : \n\n --delayed : to enable delaymode right away \n --gamemode : to enable gamemode right away" );

}

// Initializes dev and uidev
int InitializeDevices( char* device_address , bool first_time ) {

	int err; // Reperesents error numbers

	// Wait until the device file is available
	printf( "Waiting for input device file to be available...\n" );
	while ( access( device_address , F_OK ) == -1 );

	// Open the device file in read only
	fd = open( device_address , O_RDONLY );
	if ( !fd ) {

		printf( "Failed to open input device file!\nMaybe you forgot to use sudo ? ( psst i need permissions! )\n" );
        return 1;

	}

	// Create a libevdev device from the file descriptor
	err = libevdev_new_from_fd( fd , &dev );
	if ( err ) {

		printf( "Failed to create a new libevdev device!\n" );
		return 1;

	}

	// Check to see if the device has keys
	if ( !libevdev_has_event_type( dev , EV_KEY ) ) {

		printf( "The given device does not have any keys!\n" );
		return 1;

	}

	// Check to see if the device is a valid keyboard
	if ( !libevdev_has_event_code( dev , EV_KEY , KEY_A ) ) printf( "The given device isn't a valid keyboard proceeding anyway!\n" );

	// Create a libevdev uinput device from a libevdev device ( this also creates a device file like /dev/event19 which we should read from )
	err = libevdev_uinput_create_from_device( dev , LIBEVDEV_UINPUT_OPEN_MANAGED , &uidev );
	if ( err ) {

		printf( "Failed to create a new virtual device!\nMaybe you forgot to use sudo ? ( psst i need permissions! )\n" );
		return 1;

	}

	// Show the devices and wait a bit to partialy fix a bug
	if ( first_time ) { printf( "\nMonitoring <%s> for chatters and writing to <%s> :)\n" , libevdev_get_name( dev ) , libevdev_uinput_get_devnode( uidev ) ); usleep( 100 * 1000 ); }

	// Grab the device so only this program can use it thus making xorg use the virtual device instead
	err = libevdev_grab( dev , LIBEVDEV_GRAB );
	if ( err )
		printf( "Faild to use the virtual device! , you can proceed manually by disabling your main device using xinput or by using %s as your input device.\n" , libevdev_uinput_get_devnode( uidev ) );
	else if ( first_time )
		printf( "Successfully started using %s as your input device.\n" , libevdev_uinput_get_devnode( uidev ) );

	return 0;

}

// Toggles gamemode on and off ( Alowing chatters for W A S D keys )
void ToggleGameMode ( int signum ) {

	gamemode = !gamemode;
	printf( "GameMode : %s\n" , gamemode ? "On" : "Off" );

}

// Toggles delay mode on and off
void ToggleDelayMode ( int signum ) {

	delaymode = !delaymode;
	printf( "DelayMode : %s\n" , delaymode ? "On" : "Off" );

}

// Holds release events and writes them after making sure its not a chatter
void WaitForChatter ( input_event event , int chatter_threshold , thread::id &waiter_id , thread::id &chattered_waiter_id ) {

	// Waits for how ever much threshold is 
	for ( int timer = 1 ; timer <= chatter_threshold ; timer += 1 ) {

		if ( waiter_id == thread::id() ) waiter_id = this_thread::get_id();

		usleep( 1 * 1000 );

	}

	// Specify that this threads work is done
	waiter_id = thread::id();

	// If there was'nt a chatter while we were waiting and delaymode is on write the release event otherwise reset the chattered_waiter_id
	if ( chattered_waiter_id != this_thread::get_id() && delaymode ) {

		libevdev_uinput_write_event( uidev , event.type , event.code , event.value );
		libevdev_uinput_write_event( uidev , EV_SYN , SYN_REPORT , 0 );

	}
	else chattered_waiter_id = thread::id();

}

// Getting elapesd time from old_time until new_time
int GetElapsedTime( timeval old_time , timeval new_time ) {

	long seconds = new_time.tv_sec  - old_time.tv_sec;
	long useconds = new_time.tv_usec - old_time.tv_usec;
	long elapesd_ms = ( ( seconds * 1000 ) + ( useconds / 1000 ) );

	return elapesd_ms;

}

// Free the memory and unlock the device
void CleanUp( int signum ) {

	printf( "\n\nCleaning up!\n" );

	if ( uidev ) libevdev_uinput_destroy( uidev );
	if ( dev ) { libevdev_grab( dev , LIBEVDEV_UNGRAB ); libevdev_free( dev ); }
	if ( fd ) close( fd );

	printf( "Exiting!\n\n" );
	_exit( 0 );

}

// Entry
int main( int argc , char* argv[] ) {

	// Making sure to clean up before exits
	signal( SIGINT , CleanUp );
	signal( SIGTERM , CleanUp );
	signal( SIGHUP , CleanUp );
	atexit( [] {CleanUp(0);} );

	// And crashes
	signal( SIGSEGV , CleanUp );
	signal( SIGABRT , CleanUp );
	signal( SIGILL , CleanUp );
	signal( SIGFPE , CleanUp );

	// Toggle modes on user signals ( USR1 , USR2 )
	signal( SIGUSR1 , ToggleGameMode );
	signal( SIGUSR2 , ToggleDelayMode );

	// Print an empty line for spacing purposes
	printf( "\n" );

	// Show the help message if no arguments are provided
	if ( argc < 3 ) { ShowHelp(); return 1; }

	// Private variables
	vector<string> flags; // Represents a list of flags ( Arguments starting with -- )
	vector<char*> arguments; // Represents a list of actual arguments ( Arguments not starting with -- )

	// Sort programs raw arguments into two categorys : arguments and flags
	for ( int i = 1 ; i < argc ; i++ ) {

		string each_arg = argv[ i ];

		if ( each_arg.substr( 0 , 2 ) == "--" ) flags.push_back( argv[ i ] );
		else arguments.push_back( argv[ i ] );

	}

	// Check if the correct flags are given if any otherwise show help message
	for ( string each_flag : flags )
		if ( each_flag != "--gamemode" && each_flag != "--delayed" ) { ShowHelp(); return 1; }

	// Check if the correct amount of arguments are given otherwise show help message
	if ( arguments.size() != 2 ) { ShowHelp(); return 1; }

	// Private variables
	input_event ev; // Reperesents dev events

	// Represents properties of each key
	struct properties {

		timeval last_up_time;
		thread waiter;
		thread::id waiter_id = thread::id();
		thread::id chattered_waiter_id;

	};

	unordered_map< int , properties > keys; // Reperesents a dictionary of keys and their properties
	int chatter_threshold = stoi( arguments[ 1 ] ); // Reperesents the delay after a keyup event in which you cant type

	// Initialize devices for the first time
	if ( InitializeDevices( arguments[ 0 ] , true ) ) return 1;

	// Start showing logs
	printf( "\nLogs : \n" );

	// Read the flags and invoke their jobs
	for ( string each_flag : flags ) {
		if ( each_flag == "--gamemode" ) ToggleGameMode( 0 );
		if ( each_flag == "--delayed" ) ToggleDelayMode( 0 );
	}

	// Main loop , writes every dev event to uidev except the chatter ones
	while ( true ) {

		// Wait for the next event , and if we cant read events try reinitializing the devices
		if ( libevdev_next_event( dev , LIBEVDEV_READ_FLAG_BLOCKING , &ev ) == -19 ) {

			// Show log
			printf( "Faild to read the device events reinitializing\n" );

			// Clearing the devices
			libevdev_uinput_destroy( uidev );
			libevdev_grab( dev , LIBEVDEV_UNGRAB );
			libevdev_free( dev );
			close( fd );

			// Reinitializing the devices
			if ( InitializeDevices( arguments[ 0 ] , false ) ) return 1;

			continue;

		}

		// If event was a key event
		if ( ev.type == EV_KEY ) {

			switch ( ev.value ) {

				// On key up
				case 0:

					// Write the event immediately if gamemode is on and the event is for the keys W A S D or the delaymode is off
					if ( ( gamemode && ( ev.code == 17 || ev.code == 30 || ev.code == 31 || ev.code == 32 ) ) || ( !delaymode ) ) {
						libevdev_uinput_write_event( uidev , ev.type , ev.code , ev.value );
					}

					// Save the event time as the last key up time
					keys[ ev.code ].last_up_time = ev.time;

					// Start a waiter for this event to either write it after making sure its not a chatter ( delaymode ) or merely just to identify chatters in the key down events 
					keys[ ev.code ].waiter = thread( WaitForChatter , ev , chatter_threshold , ref( keys[ ev.code ].waiter_id ) , ref( keys[ ev.code ].chattered_waiter_id ) );
					keys[ ev.code ].waiter.detach();

					continue;

				break;

				// On key down
				case 1:

					// Write the event immediately and dont do anything else if gamemode is on and the event is for the keys W A S D 
					if ( gamemode && ( ev.code == 17 || ev.code == 30 || ev.code == 31 || ev.code == 32 ) ) {
						libevdev_uinput_write_event( uidev , ev.type , ev.code , ev.value );
						continue;
					}

					// Block the event if its key has a waiter
					if ( keys[ ev.code ].waiter_id != thread::id() ) {

						// Tell the waiter that its delayed event is a chatter
						keys[ ev.code ].chattered_waiter_id = keys[ ev.code ].waiter_id;

						// Show logs
						printf( "Prevented %s from chattering" , libevdev_event_code_get_name( ev.type , ev.code ) );
						printf( " , chattered after %dms.\n" , GetElapsedTime( keys[ ev.code ].last_up_time , ev.time ) );

						continue;

					}

				break;

			}

		}

		// Write not filtered dev event to uidev
		libevdev_uinput_write_event( uidev , ev.type , ev.code , ev.value );

	}

	return 0;

}
