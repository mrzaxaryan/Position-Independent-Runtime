/**
 * @file image.cc
 * @brief Image Processing Utilities — Implementation
 *
 * @details Implements the Suzuki-Abe border following algorithm for contour
 * detection in binary images, binary image differencing, and block-based
 * noise removal.
 *
 * @see S. Suzuki and K. Abe, "Topological Structural Analysis of Digitized
 *      Binary Images by Border Following," CVGIP 30(1):32-46, 1985.
 */

#include "runtime/image/image.h"

// ============================================================
//  Constants
// ============================================================

static constexpr INT32 HoleBorder = 1;
static constexpr INT32 OuterBorder = 2;
static constexpr INT32 InitialVectorCapacity = 10;

// ============================================================
//  Internal dynamic vector types
// ============================================================

struct PointVector
{
	PPoint data;
	INT32 capacity;
	INT32 count;
};

struct NodeVector
{
	PContourNode data;
	INT32 capacity;
	INT32 count;
};

struct IntVector
{
	PINT32 data;
	INT32 capacity;
	INT32 count;
};

struct Point2dVector
{
	PPPoint data;
	INT32 capacity;
	INT32 count;
};

// ============================================================
//  Vector operations
// ============================================================

static VOID InitPointVector(PointVector *pv)
{
	pv->capacity = InitialVectorCapacity;
	pv->count = 0;
	pv->data = new Point[pv->capacity];
}

static VOID InitNodeVector(NodeVector *nv)
{
	nv->capacity = InitialVectorCapacity;
	nv->count = 0;
	nv->data = new ContourNode[nv->capacity];
}

static VOID InitIntVector(IntVector *iv)
{
	iv->capacity = InitialVectorCapacity;
	iv->count = 0;
	iv->data = new INT32[iv->capacity];
}

static VOID InitPoint2dVector(Point2dVector *p2v)
{
	p2v->capacity = InitialVectorCapacity;
	p2v->count = 0;
	p2v->data = new PPoint[p2v->capacity];
}

static VOID GrowPointVector(PointVector *pv)
{
	INT32 newCapacity = pv->capacity * 2;
	PPoint newData = new Point[newCapacity];
	Memory::Copy(newData, pv->data, sizeof(Point) * (USIZE)pv->count);
	delete[] pv->data;
	pv->data = newData;
	pv->capacity = newCapacity;
}

static VOID GrowNodeVector(NodeVector *nv)
{
	INT32 newCapacity = nv->capacity * 2;
	PContourNode newData = new ContourNode[newCapacity];
	Memory::Copy(newData, nv->data, sizeof(ContourNode) * (USIZE)nv->count);
	delete[] nv->data;
	nv->data = newData;
	nv->capacity = newCapacity;
}

static VOID GrowIntVector(IntVector *iv)
{
	INT32 newCapacity = iv->capacity * 2;
	PINT32 newData = new INT32[newCapacity];
	Memory::Copy(newData, iv->data, sizeof(INT32) * (USIZE)iv->count);
	delete[] iv->data;
	iv->data = newData;
	iv->capacity = newCapacity;
}

static VOID GrowPoint2dVector(Point2dVector *p2v)
{
	INT32 newCapacity = p2v->capacity * 2;
	PPPoint newData = new PPoint[newCapacity];
	Memory::Copy(newData, p2v->data, sizeof(PPoint) * (USIZE)p2v->count);
	delete[] p2v->data;
	p2v->data = newData;
	p2v->capacity = newCapacity;
}

static VOID AddPointVector(PointVector *pv, Point point)
{
	if (pv->count + 1 >= pv->capacity)
		GrowPointVector(pv);
	pv->data[pv->count] = point;
	pv->count += 1;
}

static VOID AddNodeVector(NodeVector *nv, ContourNode node)
{
	if (nv->count + 1 >= nv->capacity)
		GrowNodeVector(nv);
	nv->data[nv->count] = node;
	nv->count += 1;
}

static VOID AddIntVector(IntVector *iv, INT32 value)
{
	if (iv->count + 1 >= iv->capacity)
		GrowIntVector(iv);
	iv->data[iv->count] = value;
	iv->count += 1;
}

static VOID AddPoint2dVector(Point2dVector *p2v, PPoint pointArray)
{
	if (p2v->count + 1 >= p2v->capacity)
		GrowPoint2dVector(p2v);
	p2v->data[p2v->count] = pointArray;
	p2v->count += 1;
}

static VOID FreePointVector(PointVector *pv)
{
	if (pv->data)
	{
		delete[] pv->data;
		pv->data = nullptr;
	}
}

static VOID FreeNodeVector(NodeVector *nv)
{
	if (nv->data)
	{
		delete[] nv->data;
		nv->data = nullptr;
	}
}

static VOID FreeIntVector(IntVector *iv)
{
	if (iv->data)
	{
		delete[] iv->data;
		iv->data = nullptr;
	}
}


static VOID ResetNode(ContourNode *n)
{
	n->parent = -1;
	n->firstChild = -1;
	n->nextSibling = -1;
}

// ============================================================
//  Geometry helpers
// ============================================================

static BOOL SamePoint(Point a, Point b)
{
	return a.row == b.row && a.col == b.col;
}

/// @brief Mark which cardinal direction around center has been examined
static VOID MarkExamined(Point mark, Point center, BOOL checked[4])
{
	//    3
	//  2 x 0
	//    1
	INT32 loc = -1;
	if (mark.col > center.col)
		loc = 0;
	else if (mark.col < center.col)
		loc = 2;
	else if (mark.row > center.row)
		loc = 1;
	else if (mark.row < center.row)
		loc = 3;

	if (loc == -1)
		return;

	checked[loc] = true;
}

/// @brief Step clockwise around a pivot point
static VOID StepCW(Point *current, Point pivot)
{
	if (current->col > pivot.col)
	{
		current->col = pivot.col;
		current->row = pivot.row + 1;
	}
	else if (current->col < pivot.col)
	{
		current->col = pivot.col;
		current->row = pivot.row - 1;
	}
	else if (current->row > pivot.row)
	{
		current->col = pivot.col - 1;
		current->row = pivot.row;
	}
	else if (current->row < pivot.row)
	{
		current->col = pivot.col + 1;
		current->row = pivot.row;
	}
}

/// @brief Step counter-clockwise around a pivot point
static VOID StepCCW(Point *current, Point pivot)
{
	if (current->col > pivot.col)
	{
		current->col = pivot.col;
		current->row = pivot.row - 1;
	}
	else if (current->col < pivot.col)
	{
		current->col = pivot.col;
		current->row = pivot.row + 1;
	}
	else if (current->row > pivot.row)
	{
		current->col = pivot.col + 1;
		current->row = pivot.row;
	}
	else if (current->row < pivot.row)
	{
		current->col = pivot.col - 1;
		current->row = pivot.row;
	}
}

// ============================================================
//  Border following (Suzuki-Abe algorithm, step 3)
// ============================================================

/**
 * @brief Follow a single border starting from (row, col) with neighbor p2
 *
 * @details Implements step 3 of the Suzuki-Abe algorithm: traces the border
 * of a connected component, recording each boundary pixel.
 *
 * @param image Binary image (modified in-place with border labels)
 * @param numRows Number of image rows
 * @param numCols Number of image columns
 * @param row Starting row of the border
 * @param col Starting column of the border
 * @param p2 Initial neighbor pixel
 * @param nbd Current border sequential number and type
 * @param contourVector Output vector of contour point arrays
 * @param contourCounter Output vector of contour point counts
 */
static VOID FollowBorder(
	CHAR *image,
	INT32 numRows,
	INT32 numCols,
	INT32 row,
	INT32 col,
	Point p2,
	Border nbd,
	Point2dVector *contourVector,
	IntVector *contourCounter)
{
	Point current;
	current.row = p2.row;
	current.col = p2.col;

	Point start;
	start.row = row;
	start.col = col;

	// (3.1) Starting from p2, look CW around start for a nonzero pixel.
	// If none found, assign -NBD to the pixel and return.
	do
	{
		StepCW(&current, start);
		if (SamePoint(current, p2))
		{
			image[start.row * numCols + start.col] = (CHAR)(-nbd.seqNum);
			PPoint temp = new Point[1];
			temp[0] = start;
			AddPoint2dVector(contourVector, temp);
			AddIntVector(contourCounter, 1);
			return;
		}
	} while ((current.col >= numCols || current.row >= numRows ||
			  current.col < 0 || current.row < 0) ||
			 image[current.row * numCols + current.col] == 0);

	PointVector pointStorage;
	InitPointVector(&pointStorage);

	Point p1 = current;

	// (3.2) (i2,j2) <- (i1,j1) and (i3,j3) <- (i,j)
	Point p3 = start;
	Point p4;
	p2 = p1;
	BOOL checked[4];

	while (true)
	{
		// (3.3) Starting from the next element of p2 in CCW order,
		// examine CCW the neighborhood of p3 to find a nonzero pixel p4.
		current = p2;

		for (INT32 i = 0; i < 4; i++)
			checked[i] = false;

		do
		{
			MarkExamined(current, p3, checked);
			StepCCW(&current, p3);
		} while ((current.col >= numCols || current.row >= numRows ||
				  current.col < 0 || current.row < 0) ||
				 image[current.row * numCols + current.col] == 0);

		p4 = current;

		// (3.4) Update pixel value at p3
		if ((p3.col + 1 >= numCols || image[p3.row * numCols + p3.col + 1] == 0) && checked[0])
		{
			image[p3.row * numCols + p3.col] = (CHAR)(-nbd.seqNum);
		}
		else if (p3.col + 1 < numCols && image[p3.row * numCols + p3.col] == 1)
		{
			image[p3.row * numCols + p3.col] = (CHAR)nbd.seqNum;
		}

		AddPointVector(&pointStorage, p3);

		// (3.5) If p4 == start and p3 == p1, we have completed the border.
		if (SamePoint(start, p4) && SamePoint(p1, p3))
		{
			// Transfer ownership of point data to contour vector
			PPoint contourPoints = new Point[(USIZE)pointStorage.count];
			Memory::Copy(contourPoints, pointStorage.data, sizeof(Point) * (USIZE)pointStorage.count);
			AddPoint2dVector(contourVector, contourPoints);
			AddIntVector(contourCounter, pointStorage.count);
			FreePointVector(&pointStorage);
			return;
		}

		p2 = p3;
		p3 = p4;
	}
}

// ============================================================
//  Absolute value helper (no CRT dependency)
// ============================================================

static constexpr INT32 AbsInt(INT32 x)
{
	return x < 0 ? -x : x;
}

// ============================================================
//  Public API
// ============================================================

[[nodiscard]] Result<void, Error> ImageProcessor::FindContours(
	CHAR *image,
	INT32 numRows,
	INT32 numCols,
	PPContour contours,
	PINT32 contourCount,
	PPContourNode hierarchy,
	PINT32 hierarchyCount)
{
	Border nbd;
	Border lnbd;

	lnbd.borderType = HoleBorder;
	nbd.borderType = HoleBorder;
	nbd.seqNum = 1;

	NodeVector hierarchyVector;
	InitNodeVector(&hierarchyVector);
	ContourNode tempNode;
	ResetNode(&tempNode);
	tempNode.border = nbd;
	AddNodeVector(&hierarchyVector, tempNode);

	// Add padding so contour and hierarchy have the same offset
	Point2dVector contourVector;
	InitPoint2dVector(&contourVector);
	PPoint padding = new Point[1];
	padding[0] = {0, -1};
	AddPoint2dVector(&contourVector, padding);

	IntVector contourCounter;
	InitIntVector(&contourCounter);
	AddIntVector(&contourCounter, 1);

	Point p2;
	BOOL borderStartFound;

	for (INT32 r = 0; r < numRows; r++)
	{
		lnbd.seqNum = 1;
		lnbd.borderType = HoleBorder;
		for (INT32 c = 0; c < numCols; c++)
		{
			borderStartFound = false;

			// Phase 1: Find border
			// If f(i,j)==1 and f(i,j-1)==0, outer border starting point
			if ((image[r * numCols + c] == 1 && c - 1 < 0) ||
				(image[r * numCols + c] == 1 && image[r * numCols + c - 1] == 0))
			{
				nbd.borderType = OuterBorder;
				nbd.seqNum += 1;
				p2.row = r;
				p2.col = c - 1;
				borderStartFound = true;
			}
			// If f(i,j)>=1 and f(i,j+1)==0, hole border starting point
			else if (c + 1 < numCols &&
					 (image[r * numCols + c] >= 1 && image[r * numCols + c + 1] == 0))
			{
				nbd.borderType = HoleBorder;
				nbd.seqNum += 1;
				if (image[r * numCols + c] > 1)
				{
					lnbd.seqNum = image[r * numCols + c];
					lnbd.borderType = hierarchyVector.data[lnbd.seqNum - 1].border.borderType;
				}
				p2.row = r;
				p2.col = c + 1;
				borderStartFound = true;
			}

			if (borderStartFound)
			{
				// Phase 2: Store parent
				ResetNode(&tempNode);
				if (nbd.borderType == lnbd.borderType)
				{
					tempNode.parent = hierarchyVector.data[lnbd.seqNum - 1].parent;
					tempNode.nextSibling = hierarchyVector.data[tempNode.parent - 1].firstChild;
					hierarchyVector.data[tempNode.parent - 1].firstChild = nbd.seqNum;
					tempNode.border = nbd;
					AddNodeVector(&hierarchyVector, tempNode);
				}
				else
				{
					if (hierarchyVector.data[lnbd.seqNum - 1].firstChild != -1)
					{
						tempNode.nextSibling = hierarchyVector.data[lnbd.seqNum - 1].firstChild;
					}
					tempNode.parent = lnbd.seqNum;
					hierarchyVector.data[lnbd.seqNum - 1].firstChild = nbd.seqNum;
					tempNode.border = nbd;
					AddNodeVector(&hierarchyVector, tempNode);
				}

				// Phase 3: Follow border
				FollowBorder(image, numRows, numCols, r, c, p2, nbd,
							 &contourVector, &contourCounter);
			}

			// Phase 4: Continue to next border
			if (AbsInt(image[r * numCols + c]) > 1)
			{
				lnbd.seqNum = AbsInt(image[r * numCols + c]);
				lnbd.borderType = hierarchyVector.data[lnbd.seqNum - 1].border.borderType;
			}
		}
	}

	// Build output arrays
	INT32 totalContours = contourVector.count;

	*contourCount = totalContours;
	*contours = new Contour[(USIZE)totalContours];

	for (INT32 i = 0; i < totalContours; i++)
	{
		(*contours)[i].points = contourVector.data[i];
		(*contours)[i].count = contourCounter.data[i];
	}

	// Transfer hierarchy ownership
	*hierarchyCount = hierarchyVector.count;
	*hierarchy = new ContourNode[(USIZE)hierarchyVector.count];
	Memory::Copy(*hierarchy, hierarchyVector.data, sizeof(ContourNode) * (USIZE)hierarchyVector.count);

	// Free internal vectors (not the contour point arrays — ownership transferred)
	FreeNodeVector(&hierarchyVector);
	FreeIntVector(&contourCounter);
	// Free only the pointer array, not the pointed-to point arrays
	delete[] contourVector.data;

	return Result<void, Error>::Ok();
}

VOID ImageProcessor::CalculateBiDifference(
	PCRGB image1,
	PCRGB image2,
	UINT32 width,
	UINT32 height,
	PUINT8 biDiff)
{
	for (UINT32 i = 0; i < height; ++i)
	{
		for (UINT32 j = 0; j < width; ++j)
		{
			UINT32 idx = i * width + j;
			if (image1[idx].Red != image2[idx].Red ||
				image1[idx].Green != image2[idx].Green ||
				image1[idx].Blue != image2[idx].Blue)
			{
				biDiff[idx] = 1;
			}
			else
			{
				biDiff[idx] = 0;
			}
		}
	}
}

VOID ImageProcessor::RemoveNoise(
	PUINT8 biDiff,
	UINT32 width,
	UINT32 height)
{
	UINT32 kernelSize = height < width ? width : height;
	kernelSize = kernelSize / 32;
	if (kernelSize == 0)
		kernelSize = 1;

	for (UINT32 i = 0; i < height; i += kernelSize)
	{
		for (UINT32 j = 0; j < width; j += kernelSize)
		{
			BOOL hasChanged = false;

			for (UINT32 k = 0; k < kernelSize && i + k < height; ++k)
			{
				for (UINT32 l = 0; l < kernelSize && j + l < width; ++l)
				{
					if (biDiff[(i + k) * width + (j + l)] != 0)
					{
						hasChanged = true;
						k = l = kernelSize;
					}
				}
			}

			for (UINT32 k = 0; k < kernelSize && i + k < height; ++k)
			{
				for (UINT32 l = 0; l < kernelSize && j + l < width; ++l)
				{
					biDiff[(i + k) * width + (j + l)] = hasChanged ? 1 : 0;
				}
			}
		}
	}
}
