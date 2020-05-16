#pragma once

#include <Processor.h>

class ProcessorCreator {

  public:

  enum PROCESSOR_TYPE {SOFTCLIP};

      ProcessorCreator();

      Processor create(PROCESSOR_TYPE type);

};