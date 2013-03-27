/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

/**
 * \file Log.hpp
 *
 * Contains a more flexible logging framework than util/OutputUtil.hpp and is intended to
 * eventually replace it.
 *
 * \note
 * The functionality here is still untested; once it's stable we will remove OutputUtil and
 * replace it with this.
 */


//This is a minimal header file, so please keep includes to a minimum.
#include "conf/settings/DisableOutput.h"

#include <iostream>
#include <fstream>
#include <string>
#include <map>

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>


namespace sim_mob {


/**
 * Used to record an event that should be logged. This generally includes simulation output
 * and other saved messages (including some profiling information).
 *
 * \author Seth N. Hetu
 *
 * The general usage pattern for this class is as follows. We use the subclass "Warn"
 *   as an example, since Log() cannot be used directly.
 *
 *   \code
 *   //Do this once for each subclass of Log, at the top of Main:
 *   if ( someConfigCheck() ) { //Should logging be enabled?
 *     Warn::Init("warnings.log")
 *   } else {
 *     Warn::Ignore()
 *   }
 *
 *   //Now use it:
 *   Warn() <<"Testing: " <<x <<"," <<y <<std::endl;
 *   Warn() <<"Using a newline instead of endl is fine too.\n";
 *
 *   //Do this once, at the end of Main:
 *   Log::Done();
 *   \endcode
 *
 * If Init() or Ignore() have not been called on a particular subclass, a default, non-synchronized stream
 * will be used (usually std::cout). If Ignore() has been called, then the system will silently ignore all
 * output, and no locking will take place. Some processing time is still wasted, so we recommend using the
 * macros (e.g., WarnLine("Test:" <<x <<"\n");) in areas where performance is crucial. Static functions
 * such as Warn::IsEnabled() can be used to check if logging on a particular subclass has been Ignored.
 *
 * Once Init() has been called with reasonable input (either a file name or "<stdout>" or "<stderr>" to
 * redirect to the standard streams), then the Log subclasses function as follows. First, a mutex is
 * seized during construction of the temporary Log object. Calls to operator<< are redirected to
 * the stream's operator of the same name. Finally, when the Log temporary is destructed, the output
 * buffer is flushed and the mutex is released.
 *
 * It is functionally possible to create a Log object that does not lock, but this is considered useless,
 * since the user would have to know that locking is not required to use that function, and mutexes which
 * are only seized by one entity incur almost no overhead. If overhead is a proble, it is likely that one
 * actually intended to disable output entirely anyway.
 *
 * Logs which access the same stream are locked with the same mutex. As a result, it is not currently
 * possible to change the log location at runtime (calling Init() multiple times is risky, although it
 * is not explicitly wrong).
 *
 * \note
 * The Log class provides only basic (static) functionality; you should use objects of its subclass
 * to accomplish your logging needs. Note that all subclasses should inherit \b privately, to
 * prevent the possibility of calling Log::Done() from a subclass.
 */
class Log : private boost::noncopyable {
public:
	/**
	 * Informs the Logging subsystem that all logging has completed, and all mutexes should be destroyed.
	 * Any calls to logging functions after this point will likely throw exceptions, as the Log subclasses
	 * will still be holding a (now invalid) pointer to the mutex.
	 *
	 * \note
	 * This function should \b not be called from a subclass (e.g., Warn). We try to prevent this by using
	 * private inheritance.
	 *
	 * \todo
	 * If we rewrite to use a boost::shared_ptr, we can avoid the need for "Done", *and* potentially allow
	 * for changing stream locations at runtime.
	 */
	static void Done();

protected:
	///Used for reference; each stream has one associated mutex. The keys (ostream pointers) are not allocated;
	/// they are just pointers to existing statically-scoped objects (or cout/cerr).
	static std::map<const std::ostream*, boost::mutex*> stream_locks;

	///Helper function for subclasses: registers an ostream with stream_locks, skipping if it's already been
	///  registered. Returns the associated mutex regardless.
	static boost::mutex* RegisterStream(const std::ostream* str);

	///Helper function for subclasses: If the associated filename is not "<stdout>" or "<stderr>", then
	///  open the ofstream object passed by reference and return a pointer to it. Else, return
	///  a pointer to std::cout or std::cerr. On error (if the file can't be created), silently
	///  return a pointer to cout.
	static std::ostream* OpenStream(const std::string& path, std::ofstream& file);
};


/**
 * Logging functionality for Warnings. See the Log class for general details about how logging works.
 *
 * To be used like so:
 *
 *   \code
 *   Warn() <<"There was a warning: " <<str <<std::endl;
 *   \endcode
 *
 * ...or, if performance is critical:
 *
 *   \code
 *   WarnLine("There was a warning: " <<str <<std::endl);
 *   \endcode
 *
 * Don't forget to call Warn::Init() at the beginning of main(), and Log::Done() at the end.
 */
class Warn : private Log {
public:
	///Construct a new Warn() object. It is best to use this object immediately, by chaining to calls of operator<<.
	Warn();

	///Destroy a Warn() object. Logging is mutually exclusive on a given output stream until the Warn object
	/// has been destroyed.
	~Warn();

	///Log a given item; this simply forwards the call to operator<< of the given logger.
	///NOTE: I am assuming that return-by-reference keeps the object alive until all chained
	///      operator<<'s are done. Should check the standard on this. ~Seth
	template <typename T>
	Warn& operator<< (const T& val);

	//Multiple calls to Init() *might* work, and the system should default to cout
	// if Init() has not been called. Either way, you should plan to call Init() once.

	///Ininitialize this Log subclass. If "path" is "<stdout>" or "<stderr>", then bind to
	///std::cout or std::cerr. Else, attempt to open the file pointed to by "path".
	///On failure, bind to std::cout.
	static void Init(const std::string& path);

	///Disable logging for this Log subclass.
	static void Ignore();

	///Is this Log subclass enabled for writing? If not, calls to operator<< will be ignored.
	static bool IsEnabled();

private:
	///A pointer to the mutex (managed in Log::stream_locks) used for locking the output stream.
	static boost::mutex* log_mutex;

	///Where to send logging events. May point to std::cout, std::cerr,
	/// or a file stream located in a subclass.
	static std::ostream* log_handle;

	///The actual file used for logging. If stdout or stderr are used for logging, this
	/// file will be un-opened. After main() terminates, this file will be destroyed and
	/// closed automatically.
	static std::ofstream log_file;

	///A scoped lock on the log_mutex. May be null, in which case output is not locked.
	boost::mutex::scoped_lock* local_lock;
};



} //End sim_mob namespace



//////////////////////////////////////////////////////////////
// Macros for each Log subclass.
//////////////////////////////////////////////////////////////

//TODO




//////////////////////////////////////////////////////////////
// Template function implementation.
//////////////////////////////////////////////////////////////

template <typename T>
sim_mob::Warn& sim_mob::Warn::operator<< (const T& val)
{
	if (log_handle) {
		(*log_handle) <<val;
	}
	return *this;
}



