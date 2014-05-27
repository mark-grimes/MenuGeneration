#include "l1menu/TriggerMenu.h"

#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include "l1menu/ITrigger.h"
#include "l1menu/TriggerConstraint.h"
#include "l1menu/tools/miscellaneous.h"
#include "l1menu/tools/fileIO.h"
#include "l1menu/tools/stringManipulation.h"
#include "l1menu/tools/XMLFile.h"
#include "l1menu/tools/XMLElement.h"

//
// Need to use the l1menu namespace for the pimple because that's how it was declared
//
namespace l1menu
{
	/** @brief Pimple class to hide the private members of TriggerMenu from the header file.
	 *
	 * Note that there are non-trivial copy constructor, move constructor and assignment operators
	 * so if you add any members you almost certainly need to edit those.
	 */
	class TriggerMenuPrivateMembers
	{
	public:
		TriggerMenuPrivateMembers();
		TriggerMenuPrivateMembers( const TriggerMenuPrivateMembers& otherPimple );
		TriggerMenuPrivateMembers( TriggerMenuPrivateMembers&& otherPimple );
		l1menu::TriggerMenuPrivateMembers& operator=( const l1menu::TriggerMenuPrivateMembers& otherPimple );
		l1menu::TriggerMenuPrivateMembers& operator=( l1menu::TriggerMenuPrivateMembers&& otherPimple ) noexcept;

		TriggerTable& triggerTable_;
		std::vector< std::unique_ptr<l1menu::ITrigger> > triggers_;
		std::vector<l1menu::TriggerConstraint> triggerConstraints_; ///< always kept the same size as triggers_
		std::vector<bool> triggerResults_; ///< @brief Stores the result of each trigger for the last call of "apply"
	};
}


//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//
//     Definitions for TriggerMenuPrivateMembers. All of these are just because of the
//     non-trivial copy/move/assignment constructors/operators.
//
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

l1menu::TriggerMenuPrivateMembers::TriggerMenuPrivateMembers()
	: triggerTable_( l1menu::TriggerTable::instance() )
{
	// No operation besides the initialiser list
}

l1menu::TriggerMenuPrivateMembers::TriggerMenuPrivateMembers( const TriggerMenuPrivateMembers& otherPimple )
	: triggerTable_(otherPimple.triggerTable_)
{
	//
	// Take a copy of all the other triggers
	//
	for( std::vector< std::unique_ptr<l1menu::ITrigger> >::const_iterator iTrigger=otherPimple.triggers_.begin(); iTrigger!=otherPimple.triggers_.end(); ++iTrigger )
	{
		l1menu::ITrigger& sourceTrigger=**iTrigger;

		triggers_.push_back( std::move(triggerTable_.copyTrigger(sourceTrigger)) );
	}

	// Copy the constraints. I'm working with a concrete implementations so this is straightforward
	triggerConstraints_=otherPimple.triggerConstraints_;
	// Make sure triggerResults_ is always the same size as triggers_
	triggerResults_.resize( triggers_.size() );
}

l1menu::TriggerMenuPrivateMembers::TriggerMenuPrivateMembers( TriggerMenuPrivateMembers&& otherPimple )
	: triggerTable_(otherPimple.triggerTable_),
	  triggers_( std::move(otherPimple.triggers_) ),
	  triggerConstraints_( std::move(otherPimple.triggerConstraints_) ),
	  triggerResults_( std::move(otherPimple.triggerResults_) )
{
	// No operation besides the initialiser list
}

l1menu::TriggerMenuPrivateMembers& l1menu::TriggerMenuPrivateMembers::operator=( const l1menu::TriggerMenuPrivateMembers& otherPimple )
{
	//
	// First clean up whatever triggers I had before
	//
	triggers_.clear();

	//
	// Now copy the triggers from the other menu
	//
	for( std::vector< std::unique_ptr<l1menu::ITrigger> >::const_iterator iTrigger=otherPimple.triggers_.begin(); iTrigger!=otherPimple.triggers_.end(); ++iTrigger )
	{
		l1menu::ITrigger& sourceTrigger=**iTrigger;

		triggers_.push_back( std::move(triggerTable_.copyTrigger(sourceTrigger)) );
	}

	// Copy the constraints. I'm working with a concrete implementations so this is straightforward
	triggerConstraints_=otherPimple.triggerConstraints_;
	// Make sure triggerResults_ is always the same size as triggers_
	triggerResults_.resize( triggers_.size() );

	return *this;
}

l1menu::TriggerMenuPrivateMembers& l1menu::TriggerMenuPrivateMembers::operator=( l1menu::TriggerMenuPrivateMembers&& otherPimple ) noexcept
{
	// Can't change the triggerTable_ reference, but it should be correct anyway
	triggers_=std::move( otherPimple.triggers_ );
	triggerConstraints_=std::move( otherPimple.triggerConstraints_ );
	triggerResults_=std::move(otherPimple.triggerResults_);

	return *this;
}

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//
//     Definitions for TriggerMenu.
//
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

l1menu::TriggerMenu::TriggerMenu() : pImple_( new l1menu::TriggerMenuPrivateMembers )
{
	// No operation besides the initialiser list
}

l1menu::TriggerMenu::~TriggerMenu()
{
	// No operation since I switched from raw pointers to unique_ptr.
}

l1menu::TriggerMenu::TriggerMenu( const TriggerMenu& otherTriggerMenu )
	: pImple_( new l1menu::TriggerMenuPrivateMembers(*otherTriggerMenu.pImple_) )
{
	// No operation besides the initialiser list. Let the TriggerMenuPrivateMembers copy
	// constructor deal with the details.
}

l1menu::TriggerMenu::TriggerMenu( TriggerMenu&& otherTriggerMenu ) noexcept
	: pImple_( std::move(otherTriggerMenu.pImple_) )
{
	// No operation besides the initialiser list. Let the TriggerMenuPrivateMembers move
	// constructor deal with the details.
}

l1menu::TriggerMenu& l1menu::TriggerMenu::operator=( const l1menu::TriggerMenu& otherTriggerMenu )
{
	// Let the TriggerMenuPrivateMembers assignment operator deal with it
	*pImple_=*otherTriggerMenu.pImple_;
	return *this;
}

l1menu::TriggerMenu& l1menu::TriggerMenu::operator=( l1menu::TriggerMenu&& otherTriggerMenu ) noexcept
{
	// Let the TriggerMenuPrivateMembers move assignment operator deal with it
	pImple_=std::move(otherTriggerMenu.pImple_);
	return *this;
}

l1menu::ITrigger& l1menu::TriggerMenu::addTrigger( const std::string& triggerName )
{
	std::unique_ptr<l1menu::ITrigger> pNewTrigger=pImple_->triggerTable_.getTrigger( triggerName );
	if( pNewTrigger.get()==NULL ) throw std::range_error( "Trigger requested that does not exist" );

	pImple_->triggers_.push_back( std::move(pNewTrigger) );

	// Add an empty constraint for this trigger
	pImple_->triggerConstraints_.push_back( l1menu::TriggerConstraint() );
	// Make sure triggerResults_ is always the same size as triggers_
	pImple_->triggerResults_.resize( pImple_->triggers_.size() );
	return *pImple_->triggers_.back();
}

l1menu::ITrigger& l1menu::TriggerMenu::addTrigger( const std::string& triggerName, unsigned int version )
{
	std::unique_ptr<l1menu::ITrigger> pNewTrigger=pImple_->triggerTable_.getTrigger( triggerName, version );
	if( pNewTrigger.get()==NULL ) throw std::range_error( "Trigger requested that does not exist" );

	pImple_->triggers_.push_back( std::move(pNewTrigger) );

	// Add an empty constraint for this trigger
	pImple_->triggerConstraints_.push_back( l1menu::TriggerConstraint() );
	// Make sure triggerResults_ is always the same size as triggers_
	pImple_->triggerResults_.resize( pImple_->triggers_.size() );
	return *pImple_->triggers_.back();
}

l1menu::ITrigger& l1menu::TriggerMenu::addTrigger( const l1menu::ITriggerDescription& triggerToCopy )
{
	std::unique_ptr<l1menu::ITrigger> pNewTrigger=pImple_->triggerTable_.copyTrigger( triggerToCopy );
	if( pNewTrigger.get()==NULL ) throw std::range_error( "Trigger requested that does not exist" );

	pImple_->triggers_.push_back( std::move(pNewTrigger) );

	// Add an empty constraint for this trigger
	pImple_->triggerConstraints_.push_back( l1menu::TriggerConstraint() );
	// Make sure triggerResults_ is always the same size as triggers_
	pImple_->triggerResults_.resize( pImple_->triggers_.size() );
	return *pImple_->triggers_.back();
}

size_t l1menu::TriggerMenu::numberOfTriggers() const
{
	return pImple_->triggers_.size();
}

l1menu::ITrigger& l1menu::TriggerMenu::getTrigger( size_t position )
{
	if( position>pImple_->triggers_.size() ) throw std::range_error( "Trigger requested that does not exist in the menu" );

	return *pImple_->triggers_[position];
}

const l1menu::ITrigger& l1menu::TriggerMenu::getTrigger( size_t position ) const
{
	if( position>pImple_->triggers_.size() ) throw std::range_error( "Trigger requested that does not exist in the menu" );

	return *pImple_->triggers_[position];
}

std::unique_ptr<l1menu::ITrigger> l1menu::TriggerMenu::getTriggerCopy( size_t position ) const
{
	if( position>pImple_->triggers_.size() ) throw std::range_error( "Trigger requested that does not exist in the menu" );

	return pImple_->triggerTable_.copyTrigger(*pImple_->triggers_[position]);
}

bool l1menu::TriggerMenu::apply( const l1menu::L1TriggerDPGEvent& event ) const
{
	bool atLeastOneTriggerHasFired=false;

	for( size_t triggerNumber=0; triggerNumber<pImple_->triggers_.size(); ++triggerNumber )
	{
		bool result=pImple_->triggers_[triggerNumber]->apply(event);
//		pImple_->triggerResults_[triggerNumber]=result;
		if( result ) atLeastOneTriggerHasFired=true;
	}

	return atLeastOneTriggerHasFired;
}

l1menu::TriggerConstraint& l1menu::TriggerMenu::getTriggerConstraint( size_t position )
{
	return pImple_->triggerConstraints_.at( position );
}

const l1menu::TriggerConstraint& l1menu::TriggerMenu::getTriggerConstraint( size_t position ) const
{
	return pImple_->triggerConstraints_.at( position );
}
