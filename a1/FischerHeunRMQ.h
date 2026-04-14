/******************************************************************************
 * File: FischerHeunRMQ.h
 *
 * A range minimum query data structure implemented using the Fischer-Heun
 * structure described in class.
 */

#ifndef FischerHeunRMQ_Included
#define FischerHeunRMQ_Included
#include "SparseTableRMQ.h"

#include "RMQEntry.h"
#include <vector>
#include <unordered_map>

class FischerHeunRMQ {
public:
  /* Constructs an RMQ structure from the specified array of elements. That
   * array may be empty.
   *
   * You aren't responsible for managing the memory of the elements array
   * provided to you here. You can assume that the array will remain valid
   * throughout the lifetime of this data structure. You should not modify the
   * contents of this array, as it might be shared across multiple RMQ
   * structures, nor should you delete it.
   */
  FischerHeunRMQ(const RMQEntry* elems, std::size_t numElems);
  
  /* Frees all memory associated with this RMQ structure. */
  ~FischerHeunRMQ();

  /* Performs an RMQ over the specified range. You can assume that low <= high
   * and that the bounds are in range and don't need to do any error-handling
   * if this is not the case.
   *
   * This function should return the *index* at which the minimum value occurs,
   * rather than the minimum value itself.
   */
  std::size_t rmq(std::size_t low, std::size_t high) const;

private:
  const RMQEntry* elems;
  size_t numElems;
  size_t blockSize = 1;
  size_t numBlocks;

  // Array storing the index of the minimum element in each block.
  std::vector<std::size_t> minimum_indices;

  // Array storing the minimum value in each block.
  RMQEntry* summary;
  SparseTableRMQ* summaryRMQ;

  // Array storing the Cartesian tree number for each block.
  std::vector<size_t> block_types;

  // Lookup table mapping tree numbers to RMQ solution for that tree number.
  std::unordered_map<size_t, std::vector<std::size_t>> precomputed_tree_numbers;
  
  /* Copying is disabled. */
  FischerHeunRMQ(const FischerHeunRMQ &) = delete;
  void operator= (FischerHeunRMQ) = delete;
};


#endif
