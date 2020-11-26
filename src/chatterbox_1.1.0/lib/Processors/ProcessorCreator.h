#ifndef _PROCESSORCREATOR_H
#define _PROCESSORCREATOR_H

#include <Processor.h>

class ProcessorCreator {

  public:

  enum PROCESSOR_TYPE {SOFTCLIP};

      ProcessorCreator();

      Processor create(PROCESSOR_TYPE type);

};

#endif