// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
//	A lot of the code in this file is based off code provided at;
//		https://code.google.com/p/battlestar-tux/
//	Full credit to them!
// ===================================================================
#include "Generic\Math\Math.h"
#include "Generic\Math\Noise\SimplexNoise.h"

int SimplexNoise::m_gradiant3D[12][3] =
{
	{1,1,0}, {-1,1,0}, {1,-1,0}, {-1,-1,0},
	{1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
	{0,1,1}, {0,-1,1}, {0,1,-1}, {0,-1,-1}
};

int SimplexNoise::m_gradiant4D[32][4] =
{
	{0,1,1,1},  {0,1,1,-1},  {0,1,-1,1},  {0,1,-1,-1},
	{0,-1,1,1}, {0,-1,1,-1}, {0,-1,-1,1}, {0,-1,-1,-1},
	{1,0,1,1},  {1,0,1,-1},  {1,0,-1,1},  {1,0,-1,-1},
	{-1,0,1,1}, {-1,0,1,-1}, {-1,0,-1,1}, {-1,0,-1,-1},
	{1,1,0,1},  {1,1,0,-1},  {1,-1,0,1},  {1,-1,0,-1},
	{-1,1,0,1}, {-1,1,0,-1}, {-1,-1,0,1}, {-1,-1,0,-1},
	{1,1,1,0},  {1,1,-1,0},  {1,-1,1,0},  {1,-1,-1,0},
	{-1,1,1,0}, {-1,1,-1,0}, {-1,-1,1,0}, {-1,-1,-1,0}
};

int SimplexNoise::m_simplex[64][4] = 
{
	{0,1,2,3},{0,1,3,2},{0,0,0,0},{0,2,3,1},{0,0,0,0},{0,0,0,0},{0,0,0,0},{1,2,3,0},
	{0,2,1,3},{0,0,0,0},{0,3,1,2},{0,3,2,1},{0,0,0,0},{0,0,0,0},{0,0,0,0},{1,3,2,0},
	{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
	{1,2,0,3},{0,0,0,0},{1,3,0,2},{0,0,0,0},{0,0,0,0},{0,0,0,0},{2,3,0,1},{2,3,1,0},
	{1,0,2,3},{1,0,3,2},{0,0,0,0},{0,0,0,0},{0,0,0,0},{2,0,3,1},{0,0,0,0},{2,1,3,0},
	{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
	{2,0,1,3},{0,0,0,0},{0,0,0,0},{0,0,0,0},{3,0,1,2},{3,0,2,1},{0,0,0,0},{3,1,2,0},
	{2,1,0,3},{0,0,0,0},{0,0,0,0},{0,0,0,0},{3,1,0,2},{0,0,0,0},{3,2,0,1},{3,2,1,0}
};

SimplexNoise::SimplexNoise(int seed)
	: m_random(seed)
{
	// Generate permutations randomly.
	for (int i = 0; i < 512; i++)
	{
		if (i < 256)
		{
			m_permutations[i] = m_random.Next(0, 256);
		}

		// Everything after 256 is a copy of the first lot of permutations.
		else
		{
			m_permutations[i] = m_permutations[i - 256];
		}
	}
}

float SimplexNoise::Dot(const int* g, const float x, const float y)
{
	return g[0]*x + g[1]*y;
}

float SimplexNoise::Dot(const int* g, const float x, const float y, const float z)
{
	return g[0]*x + g[1]*y + g[2]*z;
}

float SimplexNoise::Dot(const int* g, const float x, const float y, const float z, const float w)
{
	return g[0]*x + g[1]*y + g[2]*z + g[3]*w;
}

float SimplexNoise::Raw_Sample_2D(float x, float y)
{
    // Noise contributions from the three corners
    float n0, n1, n2;

    // Skew the input space to determine which simplex cell we're in
    float F2 = 0.5f * (sqrtf(3.0f) - 1.0f);

    // Hairy factor for 2D
    float s = (x + y) * F2;
    int i = FastFloor( x + s );
    int j = FastFloor( y + s );

    float G2 = (3.0f - sqrtf(3.0f)) / 6.0f;
    float t = (i + j) * G2;

    // Unskew the cell origin back to (x,y) space
    float X0 = i-t;
    float Y0 = j-t;

    // The x,y distances from the cell origin
    float x0 = x-X0;
    float y0 = y-Y0;

    // For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
    int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
    
	// lower triangle, XY order: (0,0)->(1,0)->(1,1)
	if (x0 > y0) 
	{
		i1=1; 
		j1=0;
	} 

	// upper triangle, YX order: (0,0)->(0,1)->(1,1)
    else 
	{
		i1=0; 
		j1=1;
	} 

    // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
    // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
    // c = (3-sqrt(3))/6
    float x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
    float y1 = y0 - j1 + G2;
    float x2 = x0 - 1.0f + 2.0f * G2; // Offsets for last corner in (x,y) unskewed coords
    float y2 = y0 - 1.0f + 2.0f * G2;

    // Work out the hashed gradient indices of the three simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int gi0 = m_permutations[ii+m_permutations[jj]] % 12;
    int gi1 = m_permutations[ii+i1+m_permutations[jj+j1]] % 12;
    int gi2 = m_permutations[ii+1+m_permutations[jj+1]] % 12;

    // Calculate the contribution from the three corners
    float t0 = 0.5f - x0*x0-y0*y0;
    if (t0 < 0) 
	{
		n0 = 0.0f;
	}
    else 
	{
        t0 *= t0;
        n0 = t0 * t0 * Dot(m_gradiant3D[gi0], x0, y0); // (x,y) of grad3 used for 2D gradient
    }

    float t1 = 0.5f - x1*x1-y1*y1;
    if (t1 < 0) 
	{
		n1 = 0.0f;
	}
    else 
	{
        t1 *= t1;
        n1 = t1 * t1 * Dot(m_gradiant3D[gi1], x1, y1);
    }

    float t2 = 0.5f - x2*x2-y2*y2;
    if (t2 < 0) 
	{
		n2 = 0.0f;
	}
    else 
	{
        t2 *= t2;
        n2 = t2 * t2 * Dot(m_gradiant3D[gi2], x2, y2);
    }

    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return 70.0f * (n0 + n1 + n2);
}

float SimplexNoise::Raw_Sample_3D(float x, float y, float z)
{
    float n0, n1, n2, n3; // Noise contributions from the four corners

    // Skew the input space to determine which simplex cell we're in
    float F3 = 1.0f/3.0f;
    float s = (x+y+z)*F3; // Very nice and simple skew factor for 3D
    int i = FastFloor(x+s);
    int j = FastFloor(y+s);
    int k = FastFloor(z+s);

    float G3 = 1.0f/6.0f; // Very nice and simple unskew factor, too
    float t = (i+j+k)*G3;
    float X0 = i-t; // Unskew the cell origin back to (x,y,z) space
    float Y0 = j-t;
    float Z0 = k-t;
    float x0 = x-X0; // The x,y,z distances from the cell origin
    float y0 = y-Y0;
    float z0 = z-Z0;

    // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    // Determine which simplex we are in.
    int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
    int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords

    if (x0 >= y0)
	{
		// X Y Z order
        if (y0 >= z0) 
		{ 
			i1=1; 
			j1=0; 
			k1=0; 
			i2=1; 
			j2=1; 
			k2=0; 
		} 
        // X Z Y order
		else if (x0 >= z0) 
		{ 
			i1=1; 
			j1=0; 
			k1=0; 
			i2=1; 
			j2=0; 
			k2=1; 
		} 
		// Z X Y order
        else 
		{ 
			i1=0; 
			j1=0; 
			k1=1; 
			i2=1; 
			j2=0; 
			k2=1; 
		} 
    }
	// x0<y0
    else 
	{ 
		// Z Y X order
        if (y0 < z0) 
		{ 
			i1=0; 
			j1=0; 
			k1=1; 
			i2=0; 
			j2=1; 
			k2=1; 
		} 
		// Y Z X order
        else if (x0 < z0) 
		{ 
			i1=0; 
			j1=1; 
			k1=0; 
			i2=0; 
			j2=1; 
			k2=1; 
		} 
		// Y X Z order
        else 
		{ 
			i1=0; 
			j1=1; 
			k1=0; 
			i2=1; 
			j2=1; 
			k2=0; 
		} 
    }

    // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
    // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
    // c = 1/6.
    float x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
    float y1 = y0 - j1 + G3;
    float z1 = z0 - k1 + G3;
    float x2 = x0 - i2 + 2.0f*G3; // Offsets for third corner in (x,y,z) coords
    float y2 = y0 - j2 + 2.0f*G3;
    float z2 = z0 - k2 + 2.0f*G3;
    float x3 = x0 - 1.0f + 3.0f*G3; // Offsets for last corner in (x,y,z) coords
    float y3 = y0 - 1.0f + 3.0f*G3;
    float z3 = z0 - 1.0f + 3.0f*G3;

    // Work out the hashed gradient indices of the four simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;
    int gi0 = m_permutations[ii+m_permutations[jj+m_permutations[kk]]] % 12;
    int gi1 = m_permutations[ii+i1+m_permutations[jj+j1+m_permutations[kk+k1]]] % 12;
    int gi2 = m_permutations[ii+i2+m_permutations[jj+j2+m_permutations[kk+k2]]] % 12;
    int gi3 = m_permutations[ii+1+m_permutations[jj+1+m_permutations[kk+1]]] % 12;

    // Calculate the contribution from the four corners
    float t0 = 0.6f - x0*x0 - y0*y0 - z0*z0;
    if (t0 < 0) 
	{
		n0 = 0.0f;
	}
    else 
	{
        t0 *= t0;
        n0 = t0 * t0 * Dot(m_gradiant3D[gi0], x0, y0, z0);
    }

    float t1 = 0.6f - x1*x1 - y1*y1 - z1*z1;
    if (t1 < 0) 
	{
		n1 = 0.0f;
	}
    else 
	{
        t1 *= t1;
        n1 = t1 * t1 * Dot(m_gradiant3D[gi1], x1, y1, z1);
    }

    float t2 = 0.6f - x2*x2 - y2*y2 - z2*z2;
    if (t2 < 0) 
	{
		n2 = 0.0f;
	}
    else 
	{
        t2 *= t2;
        n2 = t2 * t2 * Dot(m_gradiant3D[gi2], x2, y2, z2);
    }

    float t3 = 0.6f - x3*x3 - y3*y3 - z3*z3;
    if (t3 < 0)
	{
		n3 = 0.0f;
	}
    else 
	{
        t3 *= t3;
        n3 = t3 * t3 * Dot(m_gradiant3D[gi3], x3, y3, z3);
    }

    // Add contributions from each corner to get the final noise value.
    // The result is scaled to stay just inside [-1,1]
    return 32.0f*(n0 + n1 + n2 + n3);
}

float SimplexNoise::Raw_Sample_4D(float x, float y, float z, float w)
{
    // The skewing and unskewing factors are hairy again for the 4D case
    float F4 = (sqrtf(5.0f)-1.0f)/4.0f;
    float G4 = (5.0f-sqrtf(5.0f))/20.0f;
    float n0, n1, n2, n3, n4; // Noise contributions from the five corners

    // Skew the (x,y,z,w) space to determine which cell of 24 simplices we're in
    float s = (x + y + z + w) * F4; // Factor for 4D skewing
    int i = FastFloor(x + s);
    int j = FastFloor(y + s);
    int k = FastFloor(z + s);
    int l = FastFloor(w + s);
    float t = (i + j + k + l) * G4; // Factor for 4D unskewing
    float X0 = i - t; // Unskew the cell origin back to (x,y,z,w) space
    float Y0 = j - t;
    float Z0 = k - t;
    float W0 = l - t;

    float x0 = x - X0; // The x,y,z,w distances from the cell origin
    float y0 = y - Y0;
    float z0 = z - Z0;
    float w0 = w - W0;

    // For the 4D case, the simplex is a 4D shape I won't even try to describe.
    // To find out which of the 24 possible simplices we're in, we need to
    // determine the magnitude ordering of x0, y0, z0 and w0.
    // The method below is a good way of finding the ordering of x,y,z,w and
    // then find the correct traversal order for the simplex we're in.
    // First, six pair-wise comparisons are performed between each possible pair
    // of the four coordinates, and the results are used to add up binary bits
    // for an integer index.
    int c1 = (x0 > y0) ? 32 : 0;
    int c2 = (x0 > z0) ? 16 : 0;
    int c3 = (y0 > z0) ? 8 : 0;
    int c4 = (x0 > w0) ? 4 : 0;
    int c5 = (y0 > w0) ? 2 : 0;
    int c6 = (z0 > w0) ? 1 : 0;
    int c = c1 + c2 + c3 + c4 + c5 + c6;

    int i1, j1, k1, l1; // The integer offsets for the second simplex corner
    int i2, j2, k2, l2; // The integer offsets for the third simplex corner
    int i3, j3, k3, l3; // The integer offsets for the fourth simplex corner

    // simplex[c] is a 4-vector with the numbers 0, 1, 2 and 3 in some order.
    // Many values of c will never occur, since e.g. x>y>z>w makes x<z, y<w and x<w
    // impossible. Only the 24 indices which have non-zero entries make any sense.
    // We use a thresholding to set the coordinates in turn from the largest magnitude.
    // The number 3 in the "simplex" array is at the position of the largest coordinate.
    i1 = m_simplex[c][0]>=3 ? 1 : 0;
    j1 = m_simplex[c][1]>=3 ? 1 : 0;
    k1 = m_simplex[c][2]>=3 ? 1 : 0;
    l1 = m_simplex[c][3]>=3 ? 1 : 0;
    // The number 2 in the "simplex" array is at the second largest coordinate.
    i2 = m_simplex[c][0]>=2 ? 1 : 0;
    j2 = m_simplex[c][1]>=2 ? 1 : 0;
    k2 = m_simplex[c][2]>=2 ? 1 : 0;
    l2 = m_simplex[c][3]>=2 ? 1 : 0;
    // The number 1 in the "simplex" array is at the second smallest coordinate.
    i3 = m_simplex[c][0]>=1 ? 1 : 0;
    j3 = m_simplex[c][1]>=1 ? 1 : 0;
    k3 = m_simplex[c][2]>=1 ? 1 : 0;
    l3 = m_simplex[c][3]>=1 ? 1 : 0;
    // The fifth corner has all coordinate offsets = 1, so no need to look that up.

    float x1 = x0 - i1 + G4; // Offsets for second corner in (x,y,z,w) coords
    float y1 = y0 - j1 + G4;
    float z1 = z0 - k1 + G4;
    float w1 = w0 - l1 + G4;
    float x2 = x0 - i2 + 2.0f*G4; // Offsets for third corner in (x,y,z,w) coords
    float y2 = y0 - j2 + 2.0f*G4;
    float z2 = z0 - k2 + 2.0f*G4;
    float w2 = w0 - l2 + 2.0f*G4;
    float x3 = x0 - i3 + 3.0f*G4; // Offsets for fourth corner in (x,y,z,w) coords
    float y3 = y0 - j3 + 3.0f*G4;
    float z3 = z0 - k3 + 3.0f*G4;
    float w3 = w0 - l3 + 3.0f*G4;
    float x4 = x0 - 1.0f + 4.0f*G4; // Offsets for last corner in (x,y,z,w) coords
    float y4 = y0 - 1.0f + 4.0f*G4;
    float z4 = z0 - 1.0f + 4.0f*G4;
    float w4 = w0 - 1.0f + 4.0f*G4;

    // Work out the hashed gradient indices of the five simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;
    int ll = l & 255;
    int gi0 = m_permutations[ii+m_permutations[jj+m_permutations[kk+m_permutations[ll]]]] % 32;
    int gi1 = m_permutations[ii+i1+m_permutations[jj+j1+m_permutations[kk+k1+m_permutations[ll+l1]]]] % 32;
    int gi2 = m_permutations[ii+i2+m_permutations[jj+j2+m_permutations[kk+k2+m_permutations[ll+l2]]]] % 32;
    int gi3 = m_permutations[ii+i3+m_permutations[jj+j3+m_permutations[kk+k3+m_permutations[ll+l3]]]] % 32;
    int gi4 = m_permutations[ii+1+m_permutations[jj+1+m_permutations[kk+1+m_permutations[ll+1]]]] % 32;

    // Calculate the contribution from the five corners
    float t0 = 0.6f - x0*x0 - y0*y0 - z0*z0 - w0*w0;
    if (t0 < 0)
	{
		n0 = 0.0f;
	}
    else 
	{
        t0 *= t0;
        n0 = t0 * t0 * Dot(m_gradiant4D[gi0], x0, y0, z0, w0);
    }

    float t1 = 0.6f - x1*x1 - y1*y1 - z1*z1 - w1*w1;
    if(t1 < 0) 
	{
		n1 = 0.0f;
	}
    else 
	{
        t1 *= t1;
        n1 = t1 * t1 * Dot(m_gradiant4D[gi1], x1, y1, z1, w1);
    }

    float t2 = 0.6f - x2*x2 - y2*y2 - z2*z2 - w2*w2;
    if (t2 < 0) 
	{
		n2 = 0.0f;
	}
    else 
	{
        t2 *= t2;
        n2 = t2 * t2 * Dot(m_gradiant4D[gi2], x2, y2, z2, w2);
    }

    float t3 = 0.6f - x3*x3 - y3*y3 - z3*z3 - w3*w3;
    if (t3 < 0) 
	{
		n3 = 0.0f;
	}
    else 
	{
        t3 *= t3;
        n3 = t3 * t3 * Dot(m_gradiant4D[gi3], x3, y3, z3, w3);
    }

    float t4 = 0.6f - x4*x4 - y4*y4 - z4*z4 - w4*w4;
    if (t4 < 0) 
	{
		n4 = 0.0f;
	}
    else 
	{
        t4 *= t4;
        n4 = t4 * t4 * Dot(m_gradiant4D[gi4], x4, y4, z4, w4);
    }

    // Sum up and scale the result to cover the range [-1,1]
    return 27.0f * (n0 + n1 + n2 + n3 + n4);
}