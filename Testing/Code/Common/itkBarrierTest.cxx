/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkBarrierTest.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif
#include "itkBarrier.h"
#include "itkMultiThreader.h"
#include "itkMutexLock.h"
#include <stdlib.h>


class BarrierTestUserData
{
public:

  itk::Barrier::Pointer m_FirstBarrier;
  itk::Barrier::Pointer m_SecondBarrier;
  unsigned int m_Counter[3];
  unsigned int m_NumberOfIterations;
  bool m_TestFailure;
  
  BarrierTestUserData( unsigned int number_of_threads)
  {
    m_TestFailure = false;
    for (unsigned int i = 0; i < 3; i++)
      { m_Counter[i] = 0; }
    m_NumberOfIterations = 50;
    m_FirstBarrier = itk::Barrier::New();
    m_SecondBarrier = itk::Barrier::New();
    m_FirstBarrier->Initialize(number_of_threads);
    m_SecondBarrier->Initialize(number_of_threads);
  }
  ~BarrierTestUserData() {}
};

ITK_THREAD_RETURN_TYPE BarrierTestIncrement( void *ptr )
{
  int threadID = ( (itk::MultiThreader::ThreadInfoStruct *)(ptr) )->ThreadID;
  BarrierTestUserData *data = static_cast<BarrierTestUserData *>(
                  ( (itk::MultiThreader::ThreadInfoStruct *)(ptr) )->UserData );

  for (unsigned int i = 0;  i < data->m_NumberOfIterations; i++)
    {
    // set the value for this iteration
    data->m_Counter[threadID] = i;
    
    // wait for all the other threads
    data->m_FirstBarrier->Wait();
    data->m_SecondBarrier->Wait();
    }
  
  return ITK_THREAD_RETURN_VALUE;
}

ITK_THREAD_RETURN_TYPE BarrierCheckIncrement( void *ptr )
{
  BarrierTestUserData *data = static_cast<BarrierTestUserData *>(
                  ( (itk::MultiThreader::ThreadInfoStruct *)(ptr) )->UserData );

  for (unsigned int i = 0; i < data->m_NumberOfIterations; i++)
    {
    // Wait for other threads to populate the m_Counter array
    data->m_FirstBarrier->Wait();

    // Check the values in the m_Counter array
    for (unsigned int j = 0; j < 3; j++ )
      {
      if (data->m_Counter[j] != i)
        {
        data->m_TestFailure = true;
        }
      }
    data->m_SecondBarrier->Wait();
    }
  
  return ITK_THREAD_RETURN_VALUE;
}

ITK_THREAD_RETURN_TYPE BarrierTestCallback( void *ptr )
{
  int threadID = ( (itk::MultiThreader::ThreadInfoStruct *)(ptr) )->ThreadID;

  if (threadID == 3)
    {
    BarrierCheckIncrement( ptr );
    }
  else
    {
    BarrierTestIncrement( ptr );
    }
  
  return ITK_THREAD_RETURN_VALUE;
}

ITK_THREAD_RETURN_TYPE BarrierSpecialTest( void *ptr )
{
  BarrierTestUserData *data = static_cast<BarrierTestUserData *>(
                  ( (itk::MultiThreader::ThreadInfoStruct *)(ptr) )->UserData );
  
  for (unsigned int j = 0; j < 1000; j++ )
    {
    data->m_FirstBarrier->Wait();
    }
  
  return ITK_THREAD_RETURN_VALUE;
}

int itkBarrierTest(int argc, char *argv[])
{
  int number_of_threads = 4;
  if (argc > 1)
    {
    number_of_threads = ::atoi(argv[1]);
    }

  BarrierTestUserData data(number_of_threads);

  try
    {  
    itk::MultiThreader::Pointer multithreader = itk::MultiThreader::New();
    multithreader->SetNumberOfThreads(number_of_threads);
    multithreader->SetSingleMethod( BarrierTestCallback, &data);
    
    for (unsigned int i = 0; i < 5; i++)
      {
      multithreader->SingleMethodExecute();
      }
    
    // perform another test
    //    multithreader->SetSingleMethod( BarrierSpecialTest, &data);
    //   multithreader->SingleMethodExecute();
    }
  catch (itk::ExceptionObject &e)
    {
    std::cerr << e << std::endl;
    return EXIT_FAILURE;
    }

  if (data.m_TestFailure == false)
    {
    std::cout << "[TEST PASSED]" << std::endl;
    return EXIT_SUCCESS;
    }
  else
    {
    std::cout << "[TEST FAILED]" << std::endl;
    return 2;
    }
}
