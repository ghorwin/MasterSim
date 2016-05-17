/*	IBK library
	Copyright (c) 2001-2016, Institut fuer Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the IBK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation 
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	This library contains derivative work based on other open-source libraries,
	see LICENSE and OTHER_LICENSES files.
*/

#ifndef IBK_TimeH
#define IBK_TimeH

#include <string>
#include <iosfwd>   // only include the iostream prototypes

namespace IBK {

/*! Encapsulates the time consisting of a reference year and the seconds since the begin of this year.
	It is mainly used for input and output of various time formats
	\code
	// Example use:
	// simulation started in 2002, on March 1st, 6 o'clock
	IBK::Time start_time(2002, 2, 0, 6*3600);
	// 50 days of simulation done
	double simduration = 50*24*3600;
	// what is our current time?
	IBK::Time current_time = sim_start + simduration;
	// print current time in human readible form
	cout << current_time.fullString() << endl;
	\endcode
	The Time class wraps seconds up to an accuracy of milliseconds.
*/
class Time {
public:
	/*! Constructor, creates an invalid time. */
	Time();

	/*! Creates a time for a reference time point.
		\param year	The year in [years] since 0 AD.
		\param sec	The time in [seconds] since midnight, January 1st of the given year.
		\warning If seconds is negative, the year and seconds are adjusted such that
				the seconds is positive again. This can lead to a different year being returned
				by year() than passed in the constructor.
	*/
	Time(int year, double sec);

	/*! Constructor taking the time in hours, mins and seconds (year is set to 0).
		Marks the time object as invalid if time is greater then 23:59:59.
		\param hour The hour of the day [0..23].
		\param minute The minute [0..59].
		\param sec The second [0..59].
		\sa isValid()
	*/
	Time(unsigned int hour, unsigned int minute, unsigned int sec);

	/*! Creates a time object from a full date and time specification.
		Marks the time object as invalid if time is greater then 23:59:59.
		\param year	The year in [years] since 0 AD.
		\param month The month of the year [0..11].
		\param day The day of the month [0..30].
		\param seconds The seconds of the day [0..24*3600-1].
		\sa isValid()
	*/
	Time(int year, unsigned int month, unsigned int day, double seconds);

	/*! Sets the time for a reference time point.
		\param year	The year in [years] since 0 AD.
		\param sec	The time in [seconds] since midnight, January 1st of the given year.
		\warning If seconds is negative, the year and seconds are adjusted such that
				the seconds is positive again. This can lead to a different year being returned
				by year() than passed in the constructor.
	*/
	void set(int year, double sec);

	/*! Sets the time from seconds since midnight January 1st (year is set to 0).
		\param seconds Seconds since midnight January 1st.
		\code
		// set time to 22 days, 5 hours, 2 minutes
		t.set(22*24*3600 + 5*3600 + 2*60);
		\endcode
	*/
	void set(double seconds);

	/*! Sets the time (year is set to zero).
		\param hour The hour of the day [0..23].
		\param minute The minute [0..59].
		\param sec The second [0..59].
	*/
	void set(unsigned int hour, unsigned int minute, unsigned int sec);

	/*! Sets the simulation time object from a full date and time specification.
		Setting an invalid date will result in an IBK::Exception.
		\param year	The year in [years] since 0 AD.
		\param month The month of the year [0..11].
		\param day The day of the month [0..30].
		\param seconds The seconds of the day [0..24*3600-1].
		\warning Leap years are not supported, the date February 29th is invalid.
	*/
	void set(int year, unsigned int month, unsigned int day, double seconds);

	/*! Returns true, if time object contains a valid time.
		Usually, this function returns always true, except if the time object
		was created with one of the static input conversion functions with invalid string data.
	*/
	bool isValid() const;

	/*!	Decomposes the year and seconds of the time object into a full date and time specification.
		\param year	The year in [years] since 0 AD.
		\param month The month of the year [0..11].
		\param day The day of the month [0..30].
		\param seconds The seconds of the day [0..24*3600-1].
	*/
	void decomposeDate(int& year, unsigned int& month, unsigned int& day, double& seconds) const;
	/*!	Decomposes the year and seconds of the simulation time object into a full date and time specification.
		\see set() for information on argument ranges.
	*/
	void decomposeTOY(unsigned int& day, unsigned int & hour, unsigned int & minutes, unsigned int& seconds) const;

	/*! Adds 'secs' seconds to the current time. */
	Time& operator+=(double secs);
	/*! Subtracts 'secs' seconds from the current time. */
	Time& operator-=(double secs) { return operator+=(-secs); }
	/*! Subtracts the time 'rhs' from the current time.
		If rhs is positioned before lhs, the result is an invalid time.
	*/
	Time& operator-=(const Time& rhs);

//	/*! Adds the time 'rhs' to the current time (time will be wrapped at a full day). */
//	Time& operator+=(const Time& rhs);

	/*! Returns the reference year (returns 0 by default, if only the time is used). */
	int year() const { return m_year; }

	/*! Returns the current hour (0 to 23) */
	unsigned int hour() const;

	/*! Returns the current minute of the hour (0 to 59) */
	unsigned int minute() const;

	/*! Returns the current second of the minute (0 to 59) */
	unsigned int seconds() const;

	/*! Returns the seconds since begin of the reference year.
		\warning	The seconds may well span several years. Use decompose() if you want to get
					the date in normal representation.
	*/
	double secondsOfYear() const { return m_sec; }


	// *** Output conversion routines ***

	/*! Returns the current time as string in the 8-character format '12:30:25'. */
	std::string toHourFormat() const;
	/*! Returns the simulation time in the 21-character format '27 Feb 2005  22:10:40'. */
	std::string toFullDateFormat() const;
	/*! Returns the simulation time in the 17-character short english format '02/27/05 22:10:40'. */
	std::string toShortDateFormat() const;

	/*! Format types for Time-Of-Year formats. */
	enum TOYFormat {
		/*! Long format: 'xxxx days, xx hours, xx minutes and xx seconds' */
		TOY_LONG,
		/*! Long format with year: 'xx years, xx days, xx hours, xx minutes and xx seconds' */
		TOY_LONG_WITH_YEAR,
		/*! Short format: 'xxxx d  xx h  xx min  xx s' */
		TOY_SHORT,
		/*! Short format with year: 'xx a xxx d xx h xx min xx s' */
		TOY_SHORT_WITH_YEAR,
		/*! Very short 12-character format: 'ddd hh:mm:ss' */
		TOY_VERY_SHORT
	};

	/*! Returns the time offset since begin of the year in TOY format.
		\param format The format to be used.
		\return String with formatted text
		\sa TOYFormat
	*/
	std::string toTOYFormat(TOYFormat format = TOY_LONG) const;


	// *** Input conversion routines ***

	/*! Converts the time from a Time-Of-Year format into a time object.
		If input string is malformed, the resulting time object is invalid.
		\sa isValid()
	*/
	static IBK::Time fromTOY(const std::string & formatted_time, TOYFormat format);

	/*! Converts the time from the full date format '27 Feb 2005  22:10:40'.
	*/
	static IBK::Time fromFullDateFormat(const std::string & formatted_time);


	// *** Utility functions related to time formatting ***

	/*! Creates a suitable output string for a time difference by automatically
		converting the value delta_t (in seconds).
		\param delta_t Time difference in [s].
		\return Returns formatted time difference as "<value> <unit>" string.
	*/
	static std::string format_time_difference(double delta_t);

	/*! Creates a suitable output string for a time difference by automatically
		converting the value delta_t (in seconds).
		\param delta_t Time difference in [s].
		\param ustr Target time unit.
		\param uniformUnitLength If true, all unit strings have 3 characters (left aligned).
		\return Returns formatted time difference as "<value> <unit>" string.
	*/
	static std::string format_time_difference(double delta_t, const std::string & ustr, bool uniformUnitLength=false);

	/*! Determines and returns a suitable time unit for expressing the time passed in seconds. */
	static const char * suitableTimeUnit(double sec);

	/*! Determines and returns a suitable time unit for expressing the speed passed in seconds/seconds. */
	static const char * suitableSpeedUnit(double secPerSec);

	/*! Returns the current date as RFC 2822 formated string if no format string is given.
		Otherwise the format string will be used according ICO-C standard function strftime.
		\warning Total length of resulting string is limited to 80 characters.
	*/
	static std::string dateRFC2822(const std::string& timeFormat = "" );

private:
	int		m_year;		///< The year [AC].
	double	m_sec;		///< The seconds since midnight January 1st of m_year.

	friend Time operator+(const Time& lhs, double secs);
	friend Time operator-(const Time& lhs, double secs);

	friend Time operator-(const Time& lhs, const Time& rhs);
};

/*! Adds a time offset to an existing time object and returns the sum. */
inline Time operator+(const Time& lhs, double secs) { return Time(lhs) += secs; }
/*! Subtracts time offset to an existing time object and returns the sum. */
inline Time operator-(const Time& lhs, double secs) { return Time(lhs) -= secs; }

/*! Subtracts one time representation from another, hereby using the second time representation as offset/time difference. */
inline Time operator-(const Time& lhs, const Time& rhs) { return Time(lhs) -= rhs; }

} // namespace IBK

/*! \file IBK_Time.h
	\brief Contains the declaration of the class Time.
*/

#endif // IBK_TimeH
