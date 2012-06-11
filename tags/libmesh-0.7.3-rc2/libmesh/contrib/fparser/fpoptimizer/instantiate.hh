/* BEGIN_EXPLICIT_INSTANTATION */

/* Call the given instantiater for each type supported in FPoptimizer */
#define FPOPTIMIZER_EXPLICITLY_INSTANTIATE(generator) \
  FUNCTIONPARSER_INSTANTIATE_D(generator) \
  FUNCTIONPARSER_INSTANTIATE_F(generator) \
  FUNCTIONPARSER_INSTANTIATE_LD(generator) \
  FUNCTIONPARSER_INSTANTIATE_LI(generator) \
  /*FUNCTIONPARSER_INSTANTIATE_MF(generator)*/ \
  /*FUNCTIONPARSER_INSTANTIATE_GI(generator)*/ \
  /*FUNCTIONPARSER_INSTANTIATE_CD(generator)*/ \
  /*FUNCTIONPARSER_INSTANTIATE_CF(generator)*/ \
  /*FUNCTIONPARSER_INSTANTIATE_CLD(generator)*/

/* END_EXPLICIT_INSTANTATION */