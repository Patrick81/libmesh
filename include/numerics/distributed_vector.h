// The libMesh Finite Element Library.
// Copyright (C) 2002-2016 Benjamin S. Kirk, John W. Peterson, Roy H. Stogner

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



#include "libmesh/libmesh_common.h"



#ifndef LIBMESH_DISTRIBUTED_VECTOR_H
#define LIBMESH_DISTRIBUTED_VECTOR_H

// Local includes
#include "libmesh/numeric_vector.h"
#include "libmesh/parallel.h"

// C++ includes
#include <vector>
#include <algorithm>
#include <limits>

namespace libMesh
{

/**
 * Distributed vector. Provides an interface for simple
 * parallel, distributed vectors. Offers some collective
 * communication capabilities.  Note that the class will
 * sill function without MPI, but only on one processor.
 * This lets us keep the parallel details behind the scenes.
 *
 * \author Benjamin S. Kirk
 * \date 2003
 */
template <typename T>
class DistributedVector libmesh_final : public NumericVector<T>
{
public:

  /**
   *  Dummy-Constructor. Dimension=0
   */
  explicit
  DistributedVector (const Parallel::Communicator & comm,
                     const ParallelType = AUTOMATIC);

  /**
   * Constructor. Set dimension to \p n and initialize all elements with zero.
   */
  explicit
  DistributedVector (const Parallel::Communicator & comm,
                     const numeric_index_type n,
                     const ParallelType ptype = AUTOMATIC);

  /**
   * Constructor. Set local dimension to \p n_local, the global dimension
   * to \p n, and initialize all elements with zero.
   */
  DistributedVector (const Parallel::Communicator & comm,
                     const numeric_index_type n,
                     const numeric_index_type n_local,
                     const ParallelType ptype = AUTOMATIC);

  /**
   * Constructor. Set local dimension to \p n_local, the global
   * dimension to \p n, but additionally reserve memory for the
   * indices specified by the \p ghost argument.
   */
  DistributedVector (const Parallel::Communicator & comm,
                     const numeric_index_type N,
                     const numeric_index_type n_local,
                     const std::vector<numeric_index_type> & ghost,
                     const ParallelType ptype = AUTOMATIC);

  /**
   * Destructor, deallocates memory. Made virtual to allow
   * for derived classes to behave properly.
   */
  ~DistributedVector ();

  /**
   * Call the assemble functions
   */
  virtual void close () libmesh_override;

  /**
   * @returns the \p DistributedVector to a pristine state.
   */
  virtual void clear () libmesh_override;

  /**
   * Set all entries to zero. Equivalent to \p v = 0, but more obvious and
   * faster.
   */
  virtual void zero () libmesh_override;

  /**
   * Creates a vector which has the same type, size and partitioning
   * as this vector, but whose data is all zero.  Returns it in an \p
   * UniquePtr.
   */
  virtual UniquePtr<NumericVector<T> > zero_clone () const libmesh_override;

  /**
   * Creates a copy of this vector and returns it in an \p UniquePtr.
   */
  virtual UniquePtr<NumericVector<T> > clone () const libmesh_override;

  /**
   * Change the dimension of the vector to \p N. The reserved memory for
   * this vector remains unchanged if possible, to make things faster, but
   * this may waste some memory, so take this in the back of your head.
   * However, if \p N==0 all memory is freed, i.e. if you want to resize
   * the vector and release the memory not needed, you have to first call
   * \p init(0) and then \p init(N). This cited behaviour is analogous
   * to that of the STL containers.
   *
   * On \p fast==false, the vector is filled by
   * zeros.
   */
  virtual void init (const numeric_index_type N,
                     const numeric_index_type n_local,
                     const bool         fast=false,
                     const ParallelType ptype=AUTOMATIC) libmesh_override;

  /**
   * call init with n_local = N,
   */
  virtual void init (const numeric_index_type N,
                     const bool         fast=false,
                     const ParallelType ptype=AUTOMATIC) libmesh_override;

  /**
   * Create a vector that holds tha local indices plus those specified
   * in the \p ghost argument.
   */
  virtual void init (const numeric_index_type /*N*/,
                     const numeric_index_type /*n_local*/,
                     const std::vector<numeric_index_type> & /*ghost*/,
                     const bool /*fast*/ = false,
                     const ParallelType = AUTOMATIC) libmesh_override;

  /**
   * Creates a vector that has the same dimension and storage type as
   * \p other, including ghost dofs.
   */
  virtual void init (const NumericVector<T> & other,
                     const bool fast = false) libmesh_override;

  /**
   * \f$U(0-N) = s\f$: fill all components.
   */
  virtual NumericVector<T> & operator= (const T s) libmesh_override;

  /**
   *  \f$U = V\f$: copy all components.
   */
  virtual NumericVector<T> & operator= (const NumericVector<T> & v) libmesh_override;

  /**
   *  \f$U = V\f$: copy all components.
   */
  DistributedVector<T> & operator= (const DistributedVector<T> & v);

  /**
   *  \f$U = V\f$: copy all components.
   */
  virtual NumericVector<T> & operator= (const std::vector<T> & v) libmesh_override;

  /**
   * @returns the minimum element in the vector.
   * In case of complex numbers, this returns the minimum
   * Real part.
   */
  virtual Real min () const libmesh_override;

  /**
   * @returns the maximum element in the vector.
   * In case of complex numbers, this returns the maximum
   * Real part.
   */
  virtual Real max () const libmesh_override;

  /**
   * @returns the sum of all values in the vector
   */
  virtual T sum() const libmesh_override;

  /**
   * @returns the \f$l_1\f$-norm of the vector, i.e.
   * the sum of the absolute values.
   */
  virtual Real l1_norm () const libmesh_override;

  /**
   * @returns the \f$l_2\f$-norm of the vector, i.e.
   * the square root of the sum of the
   * squares of the elements.
   */
  virtual Real l2_norm () const libmesh_override;

  /**
   * @returns the maximum absolute value of the
   * elements of this vector, which is the
   * \f$l_\infty\f$-norm of a vector.
   */
  virtual Real linfty_norm () const libmesh_override;

  /**
   * @returns dimension of the vector. This
   * function was formerly called \p n(), but
   * was renamed to get the \p DistributedVector class
   * closer to the C++ standard library's
   * \p std::vector container.
   */
  virtual numeric_index_type size () const libmesh_override;

  /**
   * @returns the local size of the vector
   * (index_stop-index_start)
   */
  virtual numeric_index_type local_size() const libmesh_override;

  /**
   * @returns the index of the first vector element
   * actually stored on this processor
   */
  virtual numeric_index_type first_local_index() const libmesh_override;

  /**
   * @returns the index of the last vector element
   * actually stored on this processor
   */
  virtual numeric_index_type last_local_index() const libmesh_override;

  /**
   * Access components, returns \p U(i).
   */
  virtual T operator() (const numeric_index_type i) const libmesh_override;

  /**
   * Addition operator.
   * Fast equivalent to \p U.add(1, V).
   */
  virtual NumericVector<T> & operator += (const NumericVector<T> & v) libmesh_override;

  /**
   * Subtraction operator.
   * Fast equivalent to \p U.add(-1, V).
   */
  virtual NumericVector<T> & operator -= (const NumericVector<T> & v) libmesh_override;

  /**
   * Pointwise Division operator. ie divide every entry in this vector by the entry in v
   */
  virtual NumericVector<T> & operator /= (NumericVector<T> & v) libmesh_override;

  /**
   * Replace each entry v_i of this vector by its reciprocal, 1/v_i.
   */
  virtual void reciprocal() libmesh_override;

  /**
   * Replace each entry v_i = real(v_i) + imag(v_i)
   * of this vector by its complex conjugate, real(v_i) - imag(v_i)
   */
  virtual void conjugate() libmesh_override;

  /**
   * v(i) = value
   */
  virtual void set (const numeric_index_type i, const T value) libmesh_override;

  /**
   * v(i) += value
   */
  virtual void add (const numeric_index_type i, const T value) libmesh_override;

  /**
   * \f$U(0-LIBMESH_DIM)+=s\f$.
   * Addition of \p s to all components. Note
   * that \p s is a scalar and not a vector.
   */
  virtual void add (const T s) libmesh_override;

  /**
   * \f$U+=V\f$.
   * Simple vector addition, equal to the
   * \p operator +=.
   */
  virtual void add (const NumericVector<T> & V) libmesh_override;

  /**
   * \f$U+=a*V\f$.
   * Simple vector addition, equal to the
   * \p operator +=.
   */
  virtual void add (const T a, const NumericVector<T> & v) libmesh_override;

  /**
   * We override one NumericVector<T>::add_vector() method but don't
   * want to hide the other defaults.
   */
  using NumericVector<T>::add_vector;

  /**
   * \f$U+=A*V\f$.
   * Add the product of a Sparse matrix \p A
   * and a Numeric vector \p V to this Numeric vector.
   * @e Not @e implemented.
   */
  virtual void add_vector (const NumericVector<T> &,
                           const SparseMatrix<T> &) libmesh_override
  { libmesh_not_implemented(); }

  /**
   * \f$U+=A^T*V\f$.
   * Add the product of the transpose of a Sparse matrix \p A_trans
   * and a Numeric vector \p V to this Numeric vector.
   * @e Not @e implemented.
   */
  virtual void add_vector_transpose (const NumericVector<T> &,
                                     const SparseMatrix<T> &) libmesh_override
  { libmesh_not_implemented(); }

  /**
   * Scale each element of the
   * vector by the given factor.
   */
  virtual void scale (const T factor) libmesh_override;

  /**
   * v = abs(v)... that is, each entry in v is replaced
   * by its absolute value.
   */
  virtual void abs() libmesh_override;

  /**
   * Computes the dot product, p = U.V
   */
  virtual T dot(const NumericVector<T> & V) const libmesh_override;

  /**
   * Creates a copy of the global vector in the
   * local vector \p v_local.
   */
  virtual void localize (std::vector<T> & v_local) const libmesh_override;

  /**
   * Same, but fills a \p NumericVector<T> instead of
   * a \p std::vector.
   */
  virtual void localize (NumericVector<T> & v_local) const libmesh_override;

  /**
   * Creates a local vector \p v_local containing
   * only information relevant to this processor, as
   * defined by the \p send_list.
   */
  virtual void localize (NumericVector<T> & v_local,
                         const std::vector<numeric_index_type> & send_list) const libmesh_override;

  /**
   * Fill in the local std::vector "v_local" with the global indices
   * given in "indices".  See numeric_vector.h for more details.
   */
  virtual void localize (std::vector<T> & v_local,
                         const std::vector<numeric_index_type> & indices) const libmesh_override;

  /**
   * Updates a local vector with selected values from neighboring
   * processors, as defined by \p send_list.
   */
  virtual void localize (const numeric_index_type first_local_idx,
                         const numeric_index_type last_local_idx,
                         const std::vector<numeric_index_type> & send_list) libmesh_override;

  /**
   * Creates a local copy of the global vector in
   * \p v_local only on processor \p proc_id.  By
   * default the data is sent to processor 0.  This method
   * is useful for outputting data from one processor.
   */
  virtual void localize_to_one (std::vector<T> & v_local,
                                const processor_id_type proc_id=0) const libmesh_override;

  /**
   * Computes the pointwise (i.e. component-wise) product of \p vec1
   * and \p vec2 and stores the result in \p *this.
   */
  virtual void pointwise_mult (const NumericVector<T> & vec1,
                               const NumericVector<T> & vec2) libmesh_override;

  /**
   * Swaps the vector data and metadata
   */
  virtual void swap (NumericVector<T> & v) libmesh_override;

private:

  /**
   * Actual vector datatype
   * to hold vector entries
   */
  std::vector<T> _values;

  /**
   * The global vector size
   */
  numeric_index_type _global_size;

  /**
   * The local vector size
   */
  numeric_index_type _local_size;

  /**
   * The first component stored locally
   */
  numeric_index_type _first_local_index;

  /**
   * The last component (+1) stored locally
   */
  numeric_index_type _last_local_index;
};


//--------------------------------------------------------------------------
// DistributedVector inline methods
template <typename T>
inline
DistributedVector<T>::DistributedVector (const Parallel::Communicator & comm_in,
                                         const ParallelType ptype) :
  NumericVector<T>(comm_in, ptype),
  _global_size      (0),
  _local_size       (0),
  _first_local_index(0),
  _last_local_index (0)
{
  this->_type = ptype;
}



template <typename T>
inline
DistributedVector<T>::DistributedVector (const Parallel::Communicator & comm_in,
                                         const numeric_index_type n,
                                         const ParallelType ptype)
  : NumericVector<T>(comm_in, ptype)
{
  this->init(n, n, false, ptype);
}



template <typename T>
inline
DistributedVector<T>::DistributedVector (const Parallel::Communicator & comm_in,
                                         const numeric_index_type n,
                                         const numeric_index_type n_local,
                                         const ParallelType ptype)
  : NumericVector<T>(comm_in, ptype)
{
  this->init(n, n_local, false, ptype);
}



template <typename T>
inline
DistributedVector<T>::DistributedVector (const Parallel::Communicator & comm_in,
                                         const numeric_index_type n,
                                         const numeric_index_type n_local,
                                         const std::vector<numeric_index_type> & ghost,
                                         const ParallelType ptype)
  : NumericVector<T>(comm_in, ptype)
{
  this->init(n, n_local, ghost, false, ptype);
}



template <typename T>
inline
DistributedVector<T>::~DistributedVector ()
{
  this->clear ();
}



template <typename T>
inline
void DistributedVector<T>::init (const numeric_index_type n,
                                 const numeric_index_type n_local,
                                 const bool fast,
                                 const ParallelType ptype)
{
  // This function must be run on all processors at once
  parallel_object_only();

  libmesh_assert_less_equal (n_local, n);

  if (ptype == AUTOMATIC)
    {
      if (n == n_local)
        this->_type = SERIAL;
      else
        this->_type = PARALLEL;
    }
  else
    this->_type = ptype;

  libmesh_assert ((this->_type==SERIAL && n==n_local) ||
                  this->_type==PARALLEL);

  // Clear the data structures if already initialized
  if (this->initialized())
    this->clear();

  // Initialize data structures
  _values.resize(n_local);
  _local_size  = n_local;
  _global_size = n;

  _first_local_index = 0;

#ifdef LIBMESH_HAVE_MPI

  std::vector<numeric_index_type> local_sizes (this->n_processors(), 0);

  local_sizes[this->processor_id()] = n_local;

  this->comm().sum(local_sizes);

  // _first_local_index is the sum of _local_size
  // for all processor ids less than ours
  for (processor_id_type p=0; p!=this->processor_id(); p++)
    _first_local_index += local_sizes[p];


#  ifdef DEBUG
  // Make sure all the local sizes sum up to the global
  // size, otherwise there is big trouble!
  numeric_index_type dbg_sum=0;

  for (processor_id_type p=0; p!=this->n_processors(); p++)
    dbg_sum += local_sizes[p];

  libmesh_assert_equal_to (dbg_sum, n);

#  endif

#else

  // No other options without MPI!
  if (n != n_local)
    libmesh_error_msg("ERROR:  MPI is required for n != n_local!");

#endif

  _last_local_index = _first_local_index + n_local;

  // Set the initialized flag
  this->_is_initialized = true;

  // Zero the components unless directed otherwise
  if (!fast)
    this->zero();
}


template <typename T>
inline
void DistributedVector<T>::init (const numeric_index_type n,
                                 const numeric_index_type n_local,
                                 const std::vector<numeric_index_type> & /*ghost*/,
                                 const bool fast,
                                 const ParallelType ptype)
{
  // TODO: we shouldn't ignore the ghost sparsity pattern
  this->init(n, n_local, fast, ptype);
}



/* Default implementation for solver packages for which ghosted
   vectors are not yet implemented.  */
template <class T>
void DistributedVector<T>::init (const NumericVector<T> & other,
                                 const bool fast)
{
  this->init(other.size(),other.local_size(),fast,other.type());
}



template <typename T>
inline
void DistributedVector<T>::init (const numeric_index_type n,
                                 const bool fast,
                                 const ParallelType ptype)
{
  this->init(n,n,fast,ptype);
}



template <typename T>
inline
void DistributedVector<T>::close ()
{
  libmesh_assert (this->initialized());

  this->_is_closed = true;
}



template <typename T>
inline
void DistributedVector<T>::clear ()
{
  _values.clear();

  _global_size =
    _local_size =
    _first_local_index =
    _last_local_index = 0;


  this->_is_closed = this->_is_initialized = false;
}



template <typename T>
inline
void DistributedVector<T>::zero ()
{
  libmesh_assert (this->initialized());
  libmesh_assert_equal_to (_values.size(), _local_size);
  libmesh_assert_equal_to ((_last_local_index - _first_local_index), _local_size);

  std::fill (_values.begin(),
             _values.end(),
             0.);
}



template <typename T>
inline
UniquePtr<NumericVector<T> > DistributedVector<T>::zero_clone () const
{
  NumericVector<T> * cloned_vector = new DistributedVector<T>(this->comm());
  cloned_vector->init(*this);
  return UniquePtr<NumericVector<T> >(cloned_vector);
}



template <typename T>
inline
UniquePtr<NumericVector<T> > DistributedVector<T>::clone () const
{
  NumericVector<T> * cloned_vector = new DistributedVector<T>(this->comm());
  cloned_vector->init(*this, true);
  *cloned_vector = *this;
  return UniquePtr<NumericVector<T> >(cloned_vector);
}



template <typename T>
inline
numeric_index_type DistributedVector<T>::size () const
{
  libmesh_assert (this->initialized());
  libmesh_assert_equal_to (_values.size(), _local_size);
  libmesh_assert_equal_to ((_last_local_index - _first_local_index), _local_size);

  return _global_size;
}



template <typename T>
inline
numeric_index_type DistributedVector<T>::local_size () const
{
  libmesh_assert (this->initialized());
  libmesh_assert_equal_to (_values.size(), _local_size);
  libmesh_assert_equal_to ((_last_local_index - _first_local_index), _local_size);

  return _local_size;
}



template <typename T>
inline
numeric_index_type DistributedVector<T>::first_local_index () const
{
  libmesh_assert (this->initialized());
  libmesh_assert_equal_to (_values.size(), _local_size);
  libmesh_assert_equal_to ((_last_local_index - _first_local_index), _local_size);

  return _first_local_index;
}



template <typename T>
inline
numeric_index_type DistributedVector<T>::last_local_index () const
{
  libmesh_assert (this->initialized());
  libmesh_assert_equal_to (_values.size(), _local_size);
  libmesh_assert_equal_to ((_last_local_index - _first_local_index), _local_size);

  return _last_local_index;
}



template <typename T>
inline
T DistributedVector<T>::operator() (const numeric_index_type i) const
{
  libmesh_assert (this->initialized());
  libmesh_assert_equal_to (_values.size(), _local_size);
  libmesh_assert_equal_to ((_last_local_index - _first_local_index), _local_size);
  libmesh_assert ( ((i >= first_local_index()) &&
                    (i <  last_local_index())) );

  return _values[i - _first_local_index];
}



template <typename T>
inline
void DistributedVector<T>::set (const numeric_index_type i, const T value)
{
  libmesh_assert (this->initialized());
  libmesh_assert_equal_to (_values.size(), _local_size);
  libmesh_assert_equal_to ((_last_local_index - _first_local_index), _local_size);
  libmesh_assert_less (i, size());
  libmesh_assert_less (i-first_local_index(), local_size());

  _values[i - _first_local_index] = value;
}



template <typename T>
inline
void DistributedVector<T>::add (const numeric_index_type i, const T value)
{
  libmesh_assert (this->initialized());
  libmesh_assert_equal_to (_values.size(), _local_size);
  libmesh_assert_equal_to ((_last_local_index - _first_local_index), _local_size);
  libmesh_assert_less (i, size());
  libmesh_assert_less (i-first_local_index(), local_size());

  _values[i - _first_local_index] += value;
}



template <typename T>
inline
Real DistributedVector<T>::min () const
{
  // This function must be run on all processors at once
  parallel_object_only();

  libmesh_assert (this->initialized());
  libmesh_assert_equal_to (_values.size(), _local_size);
  libmesh_assert_equal_to ((_last_local_index - _first_local_index), _local_size);

  Real local_min = _values.size() ?
    libmesh_real(_values[0]) : std::numeric_limits<Real>::max();
  for (numeric_index_type i = 1; i < _values.size(); ++i)
    local_min = std::min(libmesh_real(_values[i]), local_min);

  this->comm().min(local_min);

  return local_min;
}



template <typename T>
inline
Real DistributedVector<T>::max() const
{
  // This function must be run on all processors at once
  parallel_object_only();

  libmesh_assert (this->initialized());
  libmesh_assert_equal_to (_values.size(), _local_size);
  libmesh_assert_equal_to ((_last_local_index - _first_local_index), _local_size);

  Real local_max = _values.size() ?
    libmesh_real(_values[0]) : -std::numeric_limits<Real>::max();
  for (numeric_index_type i = 1; i < _values.size(); ++i)
    local_max = std::max(libmesh_real(_values[i]), local_max);

  this->comm().max(local_max);

  return local_max;
}


template <typename T>
inline
void DistributedVector<T>::swap (NumericVector<T> & other)
{
  DistributedVector<T> & v = cast_ref<DistributedVector<T> &>(other);

  std::swap(_global_size, v._global_size);
  std::swap(_local_size, v._local_size);
  std::swap(_first_local_index, v._first_local_index);
  std::swap(_last_local_index, v._last_local_index);

  // This should be O(1) with any reasonable STL implementation
  std::swap(_values, v._values);
}

} // namespace libMesh


#endif  // LIBMESH_DISTRIBUTED_VECTOR_H
