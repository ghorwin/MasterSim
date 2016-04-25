#ifndef IBK_NotificationHandlerH
#define IBK_NotificationHandlerH

namespace IBK {

/*! \brief This is an abstract base class for notification objects.

	It can be used to allow call backs from a library function to another
	library without the need of making one library dependend on the other.

	Re-implement this class and use it in lengthy functions to get
	feedback information during the loading/writing process.
	The following example shows a very simple implementation of a
	notification handler that writes up to a line of hash
	characters while reading the file:
	\code
	class Notify : public IBK::NotificationHandler {
	public:
		// Initializing constructor
		Notify() : counter(0) {}

		// For functions that do not support percentage notification, simply print a
		// twisting -\|/- image.
		void notify() {
			std::cout << '\r';
			switch (counter) {
				case 0 : cout << "-";
				case 1 : cout << "\";
				case 2 : cout << "|";
				case 3 : cout << "/";
				default : ; // just here to prevent compiler warnings
			}
			counter = ++counter % 4;
			std::cout.flush();
		}

		// For functions that support percentage notification, print a
		// progress bar and percentage information.
		virtual void notify(double percentage) {
			std::cout << '\r';
			std::cout << std::setw(3) << std::right << std::fixed << std::setprecision(0) << 100*percentage;
			int chars_so_far = (int)(70*percentage);
			std::cout << " " << std::string(chars_so_far, '#');
			std::cout.flush();
		}

		unsigned int counter;
	};
	\endcode
*/
class NotificationHandler {
public:
	/*! Virtual destructor. */
	virtual ~NotificationHandler() {}
	/*! Reimplement this function in derived child 'notification' objects
		to provide whatever notification operation you want to perform.
	*/
	virtual void notify() = 0;
	/*! Reimplement this function in derived child 'notification' objects
		to provide whatever notification operation you want to perform.

		The default implementation calls notify().
	*/
	virtual void notify(double ) { notify(); }
};

} // namespace IBK

#endif // IBK_NotificationHandlerH
