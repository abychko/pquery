// eTypes.hpp
#ifndef ETYPES_HPP
#define ETYPES_HPP

enum eRETCODE
  {
  eDEFAULT,
  eMASTER,
  eCHILD,
  eERROR
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
