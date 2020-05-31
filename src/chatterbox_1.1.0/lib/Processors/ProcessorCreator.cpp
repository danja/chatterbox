#include <ProcessorCreator.h>
#include <Processor.h>
#include <Softclip.h>

ProcessorCreator::ProcessorCreator() {}

Processor ProcessorCreator::create(PROCESSOR_TYPE type)
{


   if (type == SOFTCLIP)
    {
        Softclip softclip;
        return softclip;
    }


    Processor processor;
    return processor;
}


