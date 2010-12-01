/*********************************************************************
* Software License Agreement (BSD License)
* 
*  Copyright (c) 2010, Rice University
*  All rights reserved.
* 
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
* 
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Rice University nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
* 
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

/* Author: Ioan Sucan */

#ifndef OMPL_CONTROL_SPACE_INFORMATION_
#define OMPL_CONTROL_SPACE_INFORMATION_

#include "ompl/base/SpaceInformation.h"
#include "ompl/control/ControlManifold.h"
#include "ompl/control/ControlAllocator.h"
#include "ompl/control/ControlSampler.h"
#include "ompl/control/Control.h"
#include "ompl/util/ClassForward.h"

namespace ompl
{
    
    /** \brief This namespace contains sampling based planning
	routines used by planning under differential constraints */
    namespace control
    {

	/** \brief Forward declaration of ompl::control::SpaceInformation */
	ClassForward(SpaceInformation);
	
	/** \class ompl::control::SpaceInformationPtr
	    \brief A boost shared pointer wrapper for ompl::control::SpaceInformation */

	/** \brief Space information containing necessary information for planning with controls. setup() needs to be called before use. */
	class SpaceInformation : public base::SpaceInformation
	{
	public:
	    
	    /** \brief Constructor. Sets the instance of the manifold
		to plan on. */
	    SpaceInformation(const base::StateManifoldPtr &stateManifold, const ControlManifoldPtr &controlManifold) : base::SpaceInformation(stateManifold),
														       controlManifold_(controlManifold),
														       ca_(controlManifold),
														       minSteps_(0), maxSteps_(0), stepSize_(0.0)
	    {
	    }
	    	    
	    virtual ~SpaceInformation(void)
	    {
	    }
	    
	    /** \brief Get the control manifold */
	    const ControlManifoldPtr& getControlManifold(void) const
	    {
		return controlManifold_;
	    }
	    
	    /** @name Control memory management
		@{ */
	    
	    /** \brief Allocate memory for a control */
	    Control* allocControl(void) const
	    {
		return ca_.allocControl();
	    }
	    
	    /** \brief Free the memory of a control */
	    void freeControl(Control *control) const
	    {
		ca_.freeControl(control);
	    }
	    
	    /** \brief Copy a control to another */
	    void copyControl(Control *destination, const Control *source) const
	    {
		controlManifold_->copyControl(destination, source);
	    }
	    
	    /** \brief Clone a control */
	    Control* cloneControl(const Control *source) const
	    {
		Control *copy = controlManifold_->allocControl();
		controlManifold_->copyControl(copy, source);
		return copy;
	    }

	    /** \brief Get access to the control allocator. This is the
		class that is used to allocate and free controls. The
		memory is reused whenever possible. */
	    ControlAllocator& getControlAllocator(void)
	    {
		return ca_;
	    }
	    /** @} */
	
	    /** @name Topology-specific control operations (as in the control manifold) 
		@{ */
	    
	    /** \brief Print a control to a stream */
	    void printControl(const Control *control, std::ostream &out = std::cout) const
	    {
		controlManifold_->printControl(control, out);
	    }
	    
	    /** \brief Check if two controls are the same */
	    bool equalControls(const Control *control1, const Control *control2) const
	    {
		return controlManifold_->equalControls(control1, control2);
	    }
	    
	    /** \brief Make the control have no effect if it were to be applied to a state for any amount of time. */
	    void nullControl(Control *control) const
	    {
		controlManifold_->nullControl(control);
	    }
	    
	    /** @} */

	    /** @name Sampling of controls 
		@{ */
	    
	    /** \brief Allocate a control sampler */
	    ControlSamplerPtr allocControlSampler(void) const
	    {
		return controlManifold_->allocControlSampler();
	    }
	    
	    /** \brief When controls are applied to states, they are applied for a time duration that is an integer
		multiple of the stepSize, within the bounds specified by setMinMaxControlDuration() */
	    void setPropagationStepSize(double stepSize)
	    {
		stepSize_ = stepSize;
	    }
	    
	    /** \brief Propagation is performed at integer multiples of a specified step size. This function returns the value of this step size. */
	    double getPropagationStepSize(void) const
	    {
		return stepSize_;
	    }

	    /** \brief Set the minimum and maximum number of steps a control is propagated for */
	    void setMinMaxControlDuration(unsigned int minSteps, unsigned int maxSteps)
	    {
		minSteps_ = minSteps;
		maxSteps_ = maxSteps;
	    }
	    
	    /** \brief Get the minimum number of steps a control is propagated for */
	    unsigned int getMinControlDuration(void) const
	    {
		return minSteps_;
	    }

	    /** \brief Get the maximum number of steps a control is propagated for */
	    unsigned int getMaxControlDuration(void) const
	    {
		return maxSteps_;
	    }
	    /** @} */

	    /** @name Primitives for propagating the model of the system 
		@{ */

	    /** \brief Propagate the model of the system forward, starting a a given state, with a given control, for a given number of steps. 
		\param state the state to start at
		\param control the control to apply
		\param steps the number of time steps to apply the control for. Each time step is of length getPropagationStepSize()
		\param result the state at the end of the propagation */
	    void propagate(const base::State *state, const Control* control, unsigned int steps, base::State *result) const;

	    /** \brief Propagate the model of the system forward, starting at a given state, with a given control, for a given number of steps.
		Stop if a collision is found and return the number of steps actually performed without collision. If no collision is found, the returned value is 
		equal to the \e steps argument. If a collision is found after the first step, the return value is 0 and \e result = \e state.
		\param state the state to start at
		\param control the control to apply
		\param steps the maximum number of time steps to apply the control for. Each time step is of length getPropagationStepSize()
		\param result the state at the end of the propagation or the last valid state if a collision is found */
	    unsigned int propagateWhileValid(const base::State *state, const Control* control, unsigned int steps, base::State *result) const;
	    
	    /** \brief Propagate the model of the system forward, starting a a given state, with a given control, for a given number of steps. 
		\param state the state to start at
		\param control the control to apply
		\param steps the number of time steps to apply the control for. Each time step is of length getPropagationStepSize()
		\param result the set of states along the propagated motion
		\param alloc flag indicating whether memory for the states in \e result should be allocated

		\note Start state \e state is not included in \e result */
	    void propagate(const base::State *state, const Control* control, unsigned int steps, std::vector<base::State*> &result, bool alloc) const;

	    /** \brief Propagate the model of the system forward, starting at a given state, with a given control, for a given number of steps.
		Stop if a collision is found and return the number of steps actually performed without collision. If no collision is found, the returned value is 
		equal to the \e steps argument.  If a collision is found after the first step, the return value is 0 and no states are added to \e result.
		If \e alloc is false and \e result cannot store all the generated states, propagation is stopped prematurely (when \e result is full).
		The starting state (\e state) is not included in \e result. The return value of the function indicates how many states have been written to \e result.

		\param state the state to start at
		\param control the control to apply
		\param steps the maximum number of time steps to apply the control for. Each time step is of length getPropagationStepSize()
		\param result the set of states along the propagated motion (only valid states included)
		\param alloc flag indicating whether memory for the states in \e result should be allocated
	    */
	    unsigned int propagateWhileValid(const base::State *state, const Control* control, unsigned int steps, std::vector<base::State*> &result, bool alloc) const;

	    /** @} */

	    /** \brief Print information about the current instance of the state space */
	    virtual void printSettings(std::ostream &out = std::cout) const;
	    
	    /** \brief Perform additional setup tasks (run once, before use) */
	    virtual void setup(void);
	    
	protected:

	    /** \brief The manifold describing the space of controls applicable to states in the state manifold */
	    ControlManifoldPtr controlManifold_;
	    
	    /** \brief The control allocator used by allocControl() and freeControl() */
	    ControlAllocator   ca_;

	    /** \brief The minimum number of steps to apply a control for */
	    unsigned int       minSteps_;
	    
	    /** \brief The maximum number of steps to apply a control for */
	    unsigned int       maxSteps_;

	    /** \brief The actual duration of each step */
	    double             stepSize_;
	    
	};
	
    }
    
}
    
#endif
