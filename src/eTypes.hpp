
#ifndef ETYPES_HPP
#define ETYPES_HPP

enum wRETCODE
  {
  wDEFAULT,
  wMASTER,
  wCHILD,
  wERROR
  };

enum eDBTYPE
  {
  eNONE,
  eMYSQL,
  ePGSQL
  };

enum eINFILETYPE
  {
  eSQL,
  eGENLOG,
  eBINLOG,
  eUNKNOWN
  };

enum eCMDTYPE
  {
  eCONNECT,
  eINITDB,
  eQUERY,
  eEXECUTE,
  ePREPARE,
  eSTATISTICS,
  eQUIT,
  eERR
  };
#endif
