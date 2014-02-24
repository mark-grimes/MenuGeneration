#ifndef l1menu_ITriggerDescription_h
#define l1menu_ITriggerDescription_h

#include <string>
#include <vector>

// Forward declarations
namespace l1menu
{
	namespace tools
	{
		class XMLElement;
	}
}


namespace l1menu
{
	/** @brief Abstract interface for a description. This interface should give enough information to
	 * uniquely describe a specific trigger.
	 *
	 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
	 * @date 14/Oct/2013
	 */
	class ITriggerDescription
	{
	public:
		virtual ~ITriggerDescription() {}
		virtual const std::string name() const = 0;
		virtual unsigned int version() const = 0;
		virtual const std::vector<std::string> parameterNames() const = 0;
		virtual const float& parameter( const std::string& parameterName ) const = 0;
	};

} // end of namespace l1menu

#endif
