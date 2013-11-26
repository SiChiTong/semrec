#ifndef __PLUGIN_SYMBOLIC_LOG_H__
#define __PLUGIN_SYMBOLIC_LOG_H__


// System
#include <cstdlib>
#include <iostream>

// ROS
#include <ros/ros.h>

// Designators
#include <designators/CDesignator.h>
#include <designator_integration_msgs/DesignatorCommunication.h>

// Private
#include <Types.h>
#include <ForwardDeclarations.h>
#include <Plugin.h>

using namespace std;


namespace beliefstate {
  namespace plugins {
    class PluginSymbolicLog : public Plugin {
    private:
    public:
      PluginSymbolicLog();
      ~PluginSymbolicLog();
      
      virtual Result init(int argc, char** argv);
      virtual Result deinit();
      
      virtual Result cycle();
      
      virtual void consumeEvent(Event evEvent);
    };
    
  }
  
  extern "C" plugins::PluginSymbolicLog* createInstance();
  extern "C" void destroyInstance(plugins::PluginSymbolicLog* icDestroy);
}


#endif /* __PLUGIN_SYMBOLIC_LOG_H__ */
