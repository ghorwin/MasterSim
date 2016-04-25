#ifndef IBK_bitfieldH
#define IBK_bitfieldH

#include <string>

namespace IBK {

/*! Convenience function to check if a \a bitfield contains the bit with the index \a bit_idx.
	\code
	// example, check if m_equations contains BE_MOISTURE
	bool moisture_enabled = has_flag(m_equations, BE_MOISTURE);
	\endcode
	\param bitfield An or'd combination of bits.
	\param bit_idx The index of the bit to test.
*/
inline bool has_flag(unsigned int bitfield, unsigned int bit_idx) { return (bitfield & (0x1 << bit_idx)) != 0; }

/*! Convenience function to set a flag with given index \a bit_idx in a \a bitfield.
	\code
	// example, enabled moisture mass balance
	set_flag(m_equations, BE_MOISTURE);
	\endcode
	\param bitfield An or'd combination of bits.
	\param bit_idx The index of the bit to test.
	*/
inline void set_flag(unsigned int & bitfield, unsigned int bit_idx) { bitfield |= 0x1 << bit_idx; }

/*! Convenience function to set a flag with given index \a bit_idx in a \a bitfield.
	\code
	// example, enabled moisture mass balance
	clear_flag(m_equations, BE_MOISTURE);
	\endcode
	\param bitfield An or'd combination of bits.
	\param bit_idx The index of the bit to test.
	*/
inline void clear_flag(unsigned int & bitfield, unsigned int bit_idx) { bitfield &= ~ (0x1 << bit_idx); }

} // namespace IBK

/*! \file IBK_bitfield.h
	\brief Contains utility functions for index-based bitfield operations (low-level replacement
		   for std::bitset)
*/

#endif // IBK_bitfieldH
