// Dear emacs, this is -*- c++ -*-
// $Id: ISCycleBaseHist.h,v 1.1.2.1 2008-12-01 14:52:56 krasznaa Exp $

#ifndef SFRAME_CORE_ISCycleBaseHist_H
#define SFRAME_CORE_ISCycleBaseHist_H

// Forward declaration(s):
class TList;

class ISCycleBaseHist {

public:
   virtual ~ISCycleBaseHist() {}

   virtual void SetHistOutput( TList* output ) = 0;
   virtual TList* GetHistOutput() const = 0;

}; // class ISCycleBaseHist

#endif // SFRAME_CORE_ISCycleBaseHist_H
