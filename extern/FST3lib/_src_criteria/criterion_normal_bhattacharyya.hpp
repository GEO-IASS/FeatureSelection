#ifndef FSTCRITERIONNORMALBHATTACHARYYA_H
#define FSTCRITERIONNORMALBHATTACHARYYA_H

/*!======================================================================
   Feature Selection Toolbox 3 source code
   ---------------------------------------
	
   \file    criterion_normal_bhattacharyya.hpp
   \brief   Implements Bhattacharyya distance based on normal (gaussian) model to serve as feature selection criterion
   \author  Petr Somol (somol@utia.cas.cz) with collaborators, see Contacts at http://fst.utia.cz
   \date    March 2011
   \version 3.1.0.beta
   \note    FST3 was developed using gcc 4.3 and requires
   \note    \li Boost library (http://www.boost.org/, tested with versions 1.33.1 and 1.44),
   \note    \li (\e optionally) LibSVM (http://www.csie.ntu.edu.tw/~cjlin/libsvm/, 
                tested with version 3.00)
   \note    Note that LibSVM is required for SVM related tools only,
            as demonstrated in demo12t.cpp, demo23.cpp, demo25t.cpp, demo32t.cpp, etc.

*/ /* 
=========================================================================
Copyright:
  * FST3 software (with exception of any externally linked libraries) 
    is copyrighted by Institute of Information Theory and Automation (UTIA), 
    Academy of Sciences of the Czech Republic.
  * FST3 source codes as presented here do not contain code of third parties. 
    FST3 may need linkage to external libraries to exploit its functionality
    in full. For details on obtaining and possible usage restrictions 
    of external libraries follow their original sources (referenced from
    FST3 documentation wherever applicable).
  * FST3 software is available free of charge for non-commercial use. 
    Please address all inquires concerning possible commercial use 
    of FST3, or if in doubt, to FST3 maintainer (see http://fst.utia.cz)
  * Derivative works based on FST3 are permitted as long as they remain
    non-commercial only.
  * Re-distribution of FST3 software is not allowed without explicit
    consent of the copyright holder.
Disclaimer of Warranty:
  * FST3 software is presented "as is", without warranty of any kind, 
    either expressed or implied, including, but not limited to, the implied 
    warranties of merchantability and fitness for a particular purpose. 
    The entire risk as to the quality and performance of the program 
    is with you. Should the program prove defective, you assume the cost 
    of all necessary servicing, repair or correction.
Limitation of Liability:
  * The copyright holder will in no event be liable to you for damages, 
    including any general, special, incidental or consequential damages 
    arising out of the use or inability to use the code (including but not 
    limited to loss of data or data being rendered inaccurate or losses 
    sustained by you or third parties or a failure of the program to operate 
    with any other programs).
========================================================================== */

#include <boost/smart_ptr.hpp>
#include <iostream>
#include <cmath>
#include "error.hpp"
#include "global.hpp"
#include "criterion_normal.hpp"
#include "indexed_vector.hpp"
#include "indexed_matrix.hpp"

/*============== Template parameter type naming conventions ==============
--------- Numeric types: -------------------------------------------------
DATATYPE - data sample values - usually real numbers (but may be integers
          in text processing etc.)
REALTYPE - must be real numbers - for representing intermediate results of 
          calculations like mean, covariance etc.
IDXTYPE - index values for enumeration of data samples - (nonnegative) integers, 
          extent depends on numbers of samples in data
DIMTYPE - index values for enumeration of features (dimensions), or classes (not 
          class sizes) - (nonnegative) integers, usually lower extent than IDXTYPE, 
          but be aware of expressions like _classes*_features*_features ! 
          in linearized representations of feature matrices for all classes
BINTYPE - feature selection marker type - represents ca. <10 different feature 
          states (selected, deselected, sel./desel. temporarily 1st nested loop, 2nd...)
RETURNTYPE - criterion value: real value, but may be extended in future to support 
          multiple values 
--------- Class types: ---------------------------------------------------
SUBSET       - class of class type Subset 
CLASSIFIER   - class implementing interface defined in abstract class Classifier 
EVALUATOR    - class implementing interface defined in abstract class Sequential_Step 
DISTANCE     - class implementing interface defined in abstract class Distance 
DATAACCESSOR - class implementing interface defined in abstract class Data_Accessor 
INTERVALCONTAINER - class of class type TIntervaller 
CONTAINER    - STL container of class type TInterval  
========================================================================== */

namespace FST {

/*! \brief Implements Bhattacharyya distance based on normal (gaussian) model to serve as feature selection criterion 
    \note Defined for two-class problems only */
template<class RETURNTYPE, typename DATATYPE, typename REALTYPE, typename IDXTYPE, typename DIMTYPE, class SUBSET, class DATAACCESSOR>
class Criterion_Normal_Bhattacharyya : public Criterion_Normal<RETURNTYPE,DATATYPE,REALTYPE,IDXTYPE,DIMTYPE,SUBSET,DATAACCESSOR> {
public:
	typedef Criterion_Normal<RETURNTYPE,DATATYPE,REALTYPE,IDXTYPE,DIMTYPE,SUBSET,DATAACCESSOR> parent;
	typedef boost::shared_ptr<DATAACCESSOR> PDataAccessor;
	typedef boost::shared_ptr<SUBSET> PSubset;
	Criterion_Normal_Bhattacharyya() {notify("Criterion_Normal_Bhattacharyya constructor.");}
	virtual ~Criterion_Normal_Bhattacharyya() {notify("Criterion_Normal_Bhattacharyya destructor.");}

	virtual bool evaluate(RETURNTYPE &result, const PSubset sub);
	virtual bool initialize(PDataAccessor da); 

	Criterion_Normal_Bhattacharyya* clone() const;
	Criterion_Normal_Bhattacharyya* sharing_clone() const {throw fst_error("Criterion_Normal_Bhattacharyya::sharing_clone() not supported, use Criterion_Normal_Bhattacharyya::clone() instead.");}
	Criterion_Normal_Bhattacharyya* stateless_clone() const {throw fst_error("Criterion_Normal_Bhattacharyya::stateless_clone() not supported, use Criterion_Normal_Bhattacharyya::clone() instead.");}
	
	virtual std::ostream& print(std::ostream& os) const {os << "Criterion_Normal_Bhattacharyya()"; return os;}
private:
	Criterion_Normal_Bhattacharyya(const Criterion_Normal_Bhattacharyya& cnb); // copy-constructor
private:
	boost::scoped_ptr<Indexed_Vector<DATATYPE,DIMTYPE,SUBSET> > _meandif; // to store mean[i]-mean[j]
	boost::scoped_ptr<Indexed_Matrix<DATATYPE,DIMTYPE,SUBSET> > _cov12; // to store cov[i]+cov[j]
	boost::scoped_ptr<Indexed_Matrix<DATATYPE,DIMTYPE,SUBSET> > _cov12_div2; // to store (cov[i]+cov[j])/2
	boost::scoped_ptr<Indexed_Matrix<REALTYPE,DIMTYPE,SUBSET> > _invbuf; // to store matrix inversions
	boost::scoped_ptr<Indexed_Matrix<REALTYPE,DIMTYPE,SUBSET> > _LUtemp; // to store matrix LU decomposition
};

template<class RETURNTYPE, typename DATATYPE, typename REALTYPE, typename IDXTYPE, typename DIMTYPE, class SUBSET, class DATAACCESSOR>
Criterion_Normal_Bhattacharyya<RETURNTYPE,DATATYPE,REALTYPE,IDXTYPE,DIMTYPE,SUBSET,DATAACCESSOR>::Criterion_Normal_Bhattacharyya(const Criterion_Normal_Bhattacharyya& cnb) :
	Criterion_Normal<RETURNTYPE,DATATYPE,REALTYPE,IDXTYPE,DIMTYPE,SUBSET,DATAACCESSOR>(cnb)
{
	notify("Criterion_Normal_Bhattacharyya copy-constructor.");
	if(cnb._meandif) _meandif.reset(new Indexed_Vector<DATATYPE,DIMTYPE,SUBSET>(*cnb._meandif));
	if(cnb._cov12) _cov12.reset(new Indexed_Matrix<DATATYPE,DIMTYPE,SUBSET>(*cnb._cov12));
	if(cnb._cov12_div2) _cov12_div2.reset(new Indexed_Matrix<DATATYPE,DIMTYPE,SUBSET>(*cnb._cov12_div2));
	if(cnb._invbuf) _invbuf.reset(new Indexed_Matrix<DATATYPE,DIMTYPE,SUBSET>(*cnb._invbuf));
	if(cnb._LUtemp) _LUtemp.reset(new Indexed_Matrix<DATATYPE,DIMTYPE,SUBSET>(*cnb._LUtemp));
}

template<class RETURNTYPE, typename DATATYPE, typename REALTYPE, typename IDXTYPE, typename DIMTYPE, class SUBSET, class DATAACCESSOR>
Criterion_Normal_Bhattacharyya<RETURNTYPE,DATATYPE,REALTYPE,IDXTYPE,DIMTYPE,SUBSET,DATAACCESSOR>* Criterion_Normal_Bhattacharyya<RETURNTYPE,DATATYPE,REALTYPE,IDXTYPE,DIMTYPE,SUBSET,DATAACCESSOR>::clone() const
{
	Criterion_Normal_Bhattacharyya<RETURNTYPE,DATATYPE,REALTYPE,IDXTYPE,DIMTYPE,SUBSET,DATAACCESSOR> *clone=new Criterion_Normal_Bhattacharyya<RETURNTYPE,DATATYPE,REALTYPE,IDXTYPE,DIMTYPE,SUBSET,DATAACCESSOR>(*this);
	clone->set_cloned();
	return clone;
}

template<class RETURNTYPE, typename DATATYPE, typename REALTYPE, typename IDXTYPE, typename DIMTYPE, class SUBSET, class DATAACCESSOR>
bool Criterion_Normal_Bhattacharyya<RETURNTYPE,DATATYPE,REALTYPE,IDXTYPE,DIMTYPE,SUBSET,DATAACCESSOR>::evaluate(RETURNTYPE &result, PSubset sub)
{
	notify("Criterion_Normal_Bhattacharyya::evaluate().");
	assert(parent::_model);
	assert(parent::get_n()>0);
	assert(_meandif);
	assert(_cov12);
	assert(_cov12_div2);
	assert(_invbuf);
	assert(_LUtemp);
	assert(parent::get_n()==_meandif->get_n());
	assert(parent::get_n()==_cov12->get_n());
	assert(parent::get_n()==_cov12_div2->get_n());
	assert(parent::get_n()<=_invbuf->get_n_max());
	assert(parent::get_n()<=_LUtemp->get_n_max());
	assert(sub);
	
	if(sub->get_d_raw()==0) return false;

	RETURNTYPE det1,det2,det3;
	
	parent::_model->narrow_to(sub);
	_meandif->narrow_to(sub);
	_cov12->narrow_to(sub);
	_cov12_div2->narrow_to(sub);
	
	_invbuf->redim(parent::get_d());
	_LUtemp->redim(parent::get_d());

	_cov12->LUdecompose(*_LUtemp);
	_cov12->invert(*_invbuf,*_LUtemp);
	parent::_model->get_cov()[0].LUdecompose(*_LUtemp);
	det1=parent::_model->get_cov()[0].determinant(*_LUtemp);
	parent::_model->get_cov()[1].LUdecompose(*_LUtemp);
	det2=parent::_model->get_cov()[1].determinant(*_LUtemp);
	_cov12_div2->LUdecompose(*_LUtemp);
	det3=_cov12_div2->determinant(*_LUtemp);

	assert(det1*det2!=0); // NOTE: correct handling of such stuations to be implemented !
	if(det1*det2==0) throw FST::fst_error("Criterion_Normal_Bhattacharyya::evaluate: division by zero det1*det2");
	RETURNTYPE right=0.5*log(fabs(det3)/sqrt(fabs(det1*det2)));
	RETURNTYPE left=0.0;
	RETURNTYPE pom;
	
	for(DIMTYPE j=0;j<parent::get_d();j++)
	{
		pom=0.0;
		for(DIMTYPE i=0;i<parent::get_d();i++)
		{
			pom+=_meandif->at(i)*_invbuf->at_raw(i,j);
		}
		left+=pom*_meandif->at(j);
	}
	result=0.25*left+right;
	return true;
}

template<class RETURNTYPE, typename DATATYPE, typename REALTYPE, typename IDXTYPE, typename DIMTYPE, class SUBSET, class DATAACCESSOR>
bool Criterion_Normal_Bhattacharyya<RETURNTYPE,DATATYPE,REALTYPE,IDXTYPE,DIMTYPE,SUBSET,DATAACCESSOR>::initialize(PDataAccessor da)
{
	if(da->getNoOfClasses()!=2) throw fst_error("Criterion_Normal_Bhattacharyya() defined for two-class problems only.");
	notify("Criterion_Normal_Bhattacharyya::initialize().");
	parent::initialize(da);
		
	if(!_meandif) _meandif.reset(new Indexed_Vector<DATATYPE,DIMTYPE,SUBSET>);
	if(parent::get_n()>_meandif->get_n()) _meandif->reset(parent::get_n());
	if(!_cov12) _cov12.reset(new Indexed_Matrix<DATATYPE,DIMTYPE,SUBSET>);
	if(parent::get_n()>_cov12->get_n()) _cov12->reset(parent::get_n());
	if(!_cov12_div2) _cov12_div2.reset(new Indexed_Matrix<DATATYPE,DIMTYPE,SUBSET>);
	if(parent::get_n()>_cov12_div2->get_n()) _cov12_div2->reset(parent::get_n());
	if(!_invbuf) _invbuf.reset(new Indexed_Matrix<REALTYPE,DIMTYPE,SUBSET>(parent::get_n()));
	if(parent::get_n()>_invbuf->get_n()) _invbuf->reset(parent::get_n());
	if(!_LUtemp) _LUtemp.reset(new Indexed_Matrix<REALTYPE,DIMTYPE,SUBSET>(parent::get_n(),true));
	if(parent::get_n()>_LUtemp->get_n()) _LUtemp->reset(parent::get_n(),true);

	_meandif->copy_raw(parent::_model->get_mean()[0]);
	_meandif->subtract_raw(parent::_model->get_mean()[1]);
	_cov12->copy_raw(parent::_model->get_cov()[0]);
	_cov12->add_raw(parent::_model->get_cov()[1]);
	_cov12_div2->copy_constmul_raw(*_cov12,0.5);
		
	return true; 
}


} // namespace
#endif // FSTCRITERIONNORMALBHATTACHARYYA_H ///:~
