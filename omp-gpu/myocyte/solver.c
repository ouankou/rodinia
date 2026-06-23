#include <math.h>
#include <stddef.h>

#define max(x,y) ( (x) < (y) ? (y) : (x) )
#define min(x,y) ( (x) < (y) ? (x) : (y) )

#define ATTEMPTS 12
#define MIN_SCALE_FACTOR 0.125f
#define MAX_SCALE_FACTOR 4.0f

int solver(fp *y, int xmax, fp *params, int mode, fp *workspace)
{
	fp err_exponent;
	fp h_init;
	fp tolerance;
	int xmin;

	err_exponent = 1.0f / 7.0f;
	h_init = 1;
	xmin = 0;
	tolerance = 10 / (fp)(xmax - xmin);

	if (xmax < xmin || h_init <= 0.0f) {
		return -2;
	}

	if (xmax == xmin) {
		return 0;
	}

	for (int k = 1; k <= xmax; k++) {
		fp x = k - 1;
		fp h = h_init;
		fp scale_fina = 1.0f;
		fp err[EQUATIONS];
		fp scale[EQUATIONS];
		fp yy[EQUATIONS];
		int j;

		for (j = 0; j < ATTEMPTS; j++) {
			int error = 0;
			int outside = 0;
			fp scale_min = MAX_SCALE_FACTOR;

			embedded_fehlberg_7_8(x,
								  h,
								  y + (size_t)(k - 1) * EQUATIONS,
								  y + (size_t)k * EQUATIONS,
								  err,
								  params,
								  mode,
								  workspace);

			for (int i = 0; i < EQUATIONS; i++) {
				if (err[i] > 0) {
					error = 1;
				}
			}
			if (error != 1) {
				scale_fina = MAX_SCALE_FACTOR;
				break;
			}

			for (int i = 0; i < EQUATIONS; i++) {
				fp previous_y = y[(size_t)(k - 1) * EQUATIONS + i];
				if (previous_y == 0.0f) {
					yy[i] = tolerance;
				} else {
					yy[i] = fabsf(previous_y);
				}
				scale[i] = 0.8f * powf(tolerance * yy[i] / err[i], err_exponent);
				if (scale[i] < scale_min) {
					scale_min = scale[i];
				}
			}
			scale_fina = min(max(scale_min, MIN_SCALE_FACTOR), MAX_SCALE_FACTOR);

			for (int i = 0; i < EQUATIONS; i++) {
				if (err[i] > (tolerance * yy[i])) {
					outside = 1;
				}
			}
			if (outside == 0) {
				break;
			}

			h = h * scale_fina;

			if (h >= 0.9f) {
				h = 0.9f;
			}

			if (x + h > (fp)xmax) {
				h = (fp)xmax - x;
			} else if (x + h + 0.5f * h > (fp)xmax) {
				h = 0.5f * h;
			}
		}

		if (j >= ATTEMPTS) {
			return -1;
		}
	}

	return 0;
}
