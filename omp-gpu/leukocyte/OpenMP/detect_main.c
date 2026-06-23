#include "find_ellipse.h"
#include "track_ellipse.h"

int main(int argc, char ** argv) {

	// Keep track of the start time of the program
  long long program_start_time = get_time();

	// Let the user specify the number of frames to process
	int num_frames = 1;

	if (argc !=3){
		fprintf(stderr, "usage: %s <num of frames> <input file>\n", argv[0]);
		exit(1);
	}

	if (argc > 1){
		num_frames = atoi(argv[1]);
		}
	// Open video file
	char *video_file_name;
	video_file_name = argv[2];

	avi_t *cell_file = AVI_open_input_file(video_file_name, 1);
	if (cell_file == NULL)	{
		AVI_print_error("Error with AVI_open_input_file");
		return -1;
	}

	int i, j, Iter = 20, ns = 4, k_count = 0, n;
	int *crow = NULL, *ccol = NULL;
	size_t pair_counter = 0;
	size_t x_result_len = 0;
	MAT *cellx = NULL, *celly = NULL, *A = NULL;
	MAT *grad_x = NULL, *grad_y = NULL, *gicov = NULL;
	MAT *max_gicov = NULL, *strel = NULL, *img_dilated = NULL;
	double *GICOV_spots = NULL, *t = NULL, *G = NULL, *x_result = NULL, *y_result = NULL;
	double *V = NULL, *QAX_CENTERS = NULL, *QAY_CENTERS = NULL;
	double threshold = 1.8, radius = 10.0, delta = 3.0, dt = 0.01, b = 5.0;

	// Extract a cropped version of the first frame from the video file
	MAT *image_chopped = get_frame(cell_file, 0, 1, 0);
	printf("Detecting cells in frame 0\n");

	// Get gradient matrices in x and y directions
	grad_x = gradient_x(image_chopped);
	grad_y = gradient_y(image_chopped);

	m_free(image_chopped);

	// Get GICOV matrix corresponding to image gradients
	long long GICOV_start_time = get_time();
	gicov = ellipsematching(grad_x, grad_y);

	// Square GICOV values
	max_gicov = m_get(gicov->m, gicov->n);
	for (i = 0; i < gicov->m; i++) {
		for (j = 0; j < gicov->n; j++) {
			double val = m_get_val(gicov, i, j);
			m_set_val(max_gicov, i, j, val * val);
		}
	}

	long long GICOV_end_time = get_time();

	// Dilate the GICOV matrix
	long long dilate_start_time = get_time();
	strel = structuring_element(12);
	img_dilated = dilate_f(max_gicov, strel);
	long long dilate_end_time = get_time();

	// Find possible matches for cell centers based on GICOV and record the rows/columns in which they are found
	pair_counter = 0;
	size_t spot_capacity;
	if ((size_t)max_gicov->m != 0 &&
	    (size_t)max_gicov->n > ((size_t)-1) / (size_t)max_gicov->m) {
		fprintf(stderr, "Error: GICOV matrix is too large\n");
		goto cleanup_failure;
	}
	spot_capacity = (size_t)max_gicov->m * (size_t)max_gicov->n;
	if (spot_capacity > ((size_t)-1) / sizeof(int)) {
		fprintf(stderr, "Error: GICOV coordinate array is too large\n");
		goto cleanup_failure;
	}
	crow = (int *) malloc(spot_capacity * sizeof(int));
	ccol = (int *) malloc(spot_capacity * sizeof(int));
	if (crow == NULL || ccol == NULL) {
		fprintf(stderr, "Error: Memory allocation failed for GICOV coordinates\n");
		goto cleanup_failure;
	}
	for (i = 0; i < max_gicov->m; i++) {
		for (j = 0; j < max_gicov->n; j++) {
			if (!(m_get_val(max_gicov,i,j) == 0.0) && (m_get_val(img_dilated,i,j) == m_get_val(max_gicov,i,j))) {
				crow[pair_counter] = i;
				ccol[pair_counter] = j;
				pair_counter++;
			}
		}
	}

	if (pair_counter > ((size_t)-1) / sizeof(double)) {
		fprintf(stderr, "Error: GICOV spot array is too large\n");
		goto cleanup_failure;
	}
	GICOV_spots = (double *) malloc(sizeof(double)*pair_counter);
	if (pair_counter != 0 && GICOV_spots == NULL) {
		fprintf(stderr, "Error: Memory allocation failed for GICOV spots\n");
		goto cleanup_failure;
	}
	for (size_t idx = 0; idx < pair_counter; idx++)
		GICOV_spots[idx] = m_get_val(gicov, crow[idx], ccol[idx]);

	G = (double *) calloc(pair_counter, sizeof(double));
	x_result = (double *) calloc(pair_counter, sizeof(double));
	y_result = (double *) calloc(pair_counter, sizeof(double));
	if (pair_counter != 0 && (G == NULL || x_result == NULL || y_result == NULL)) {
		fprintf(stderr, "Error: Memory allocation failed for detection result buffers\n");
		goto cleanup_failure;
	}

	x_result_len = 0;
	for (size_t idx = 0; idx < pair_counter; idx++) {
		if ((crow[idx] > 29) && (crow[idx] < BOTTOM - TOP + 39)) {
			x_result[x_result_len] = ccol[idx];
			y_result[x_result_len] = crow[idx] - 40;
			G[x_result_len] = GICOV_spots[idx];
			x_result_len++;
		}
	}

	// Make an array t which holds each "time step" for the possible cells
	t = (double *) malloc(sizeof(double) * 36);
	if (t == NULL) {
		fprintf(stderr, "Error: Memory allocation failed for sample time buffer\n");
		goto cleanup_failure;
	}
	for (i = 0; i < 36; i++) {
		t[i] = (double)i * 2.0 * PI / 36.0;
	}

	// Store cell boundaries (as simple circles) for all cells
	cellx = m_get((int)x_result_len, 36);
	celly = m_get((int)x_result_len, 36);
	for(i = 0; i < (int)x_result_len; i++) {
		for(j = 0; j < 36; j++) {
			m_set_val(cellx, i, j, x_result[i] + radius * cos(t[j]));
			m_set_val(celly, i, j, y_result[i] + radius * sin(t[j]));
		}
	}

	A = TMatrix(9,4);


	V = (double *) calloc(pair_counter, sizeof(double));
	QAX_CENTERS = (double *) calloc(pair_counter, sizeof(double));
	QAY_CENTERS = (double *) calloc(pair_counter, sizeof(double));
	if (pair_counter != 0 && (V == NULL || QAX_CENTERS == NULL || QAY_CENTERS == NULL)) {
		fprintf(stderr, "Error: Memory allocation failed for ellipse center buffers\n");
		goto cleanup_failure;
	}

	// For all possible results, find the ones that are feasibly leukocytes and store their centers
	k_count = 0;
	for (n = 0; n < (int)x_result_len; n++) {
		if ((G[n] < -1 * threshold) || G[n] > threshold) {
			MAT * x, *y;
			VEC * x_row, * y_row;
			x = m_get(1, 36);
			y = m_get(1, 36);

			x_row = v_get(36);
			y_row = v_get(36);

			// Get current values of possible cells from cellx/celly matrices
			x_row = get_row(cellx, n, x_row);
			y_row = get_row(celly, n, y_row);
			uniformseg(x_row, y_row, x, y);

			// Make sure that the possible leukocytes are not too close to the edge of the frame
			if ((m_min(x) > b) && (m_min(y) > b) && (m_max(x) < cell_file->width - b) && (m_max(y) < cell_file->height - b)) {
				MAT * Cx, * Cy, *Cy_temp, * Ix1, * Iy1;
				VEC  *Xs, *Ys, *W, *Nx, *Ny, *X, *Y;
				Cx = m_get(1, 36);
				Cy = m_get(1, 36);
				Cx = mmtr_mlt(A, x, Cx);
				Cy = mmtr_mlt(A, y, Cy);

				Cy_temp = m_get(Cy->m, Cy->n);

				for (i = 0; i < 9; i++)
					m_set_val(Cy, i, 0, m_get_val(Cy, i, 0) + 40.0);

				// Iteratively refine the snake/spline
				for (i = 0; i < Iter; i++) {
					int typeofcell;

					if(G[n] > 0.0) typeofcell = 0;
					else typeofcell = 1;

					splineenergyform01(Cx, Cy, grad_x, grad_y, ns, delta, 2.0 * dt, typeofcell);
				}

				X = getsampling(Cx, ns);
				for (i = 0; i < Cy->m; i++)
					m_set_val(Cy_temp, i, 0, m_get_val(Cy, i, 0) - 40.0);
				Y = getsampling(Cy_temp, ns);

				Ix1 = linear_interp2(grad_x, X, Y);
				Iy1 = linear_interp2(grad_y, X, Y);
				Xs = getfdriv(Cx, ns);
				Ys = getfdriv(Cy, ns);

				Nx = v_get(Ys->dim);
				for (i = 0; i < Ys->dim; i++)
					v_set_val(Nx, i, v_get_val(Ys, i) / sqrt(v_get_val(Xs, i)*v_get_val(Xs, i) + v_get_val(Ys, i)*v_get_val(Ys, i)));

				Ny = v_get(Xs->dim);
				for (i = 0; i < Xs->dim; i++)
					v_set_val(Ny, i, -1.0 * v_get_val(Xs, i) / sqrt(v_get_val(Xs, i)*v_get_val(Xs, i) + v_get_val(Ys, i)*v_get_val(Ys, i)));

				W = v_get(Nx->dim);
				for (i = 0; i < Nx->dim; i++)
					v_set_val(W, i, m_get_val(Ix1, 0, i) * v_get_val(Nx, i) + m_get_val(Iy1, 0, i) * v_get_val(Ny, i));

				V[n] = mean(W) / std_dev(W);

				//get means of X and Y values for all "snaxels" of the spline contour, thus finding the cell centers
				QAX_CENTERS[k_count] = mean(X);
				QAY_CENTERS[k_count] = mean(Y) + TOP;

				k_count++;

				// Free memory
				v_free(W);
				v_free(Ny);
				v_free(Nx);
				v_free(Ys);
				v_free(Xs);
				m_free(Iy1);
				m_free(Ix1);
				v_free(Y);
				v_free(X);
				m_free(Cy_temp);
				m_free(Cy);
				m_free(Cx);
			}

			// Free memory
			v_free(y_row);
			v_free(x_row);
			m_free(y);
			m_free(x);
		}
	}

	// Free memory
	free(V);
	free(ccol);
	free(crow);
	free(GICOV_spots);
	free(t);
	free(G);
	free(x_result);
	free(y_result);
	m_free(A);
	m_free(celly);
	m_free(cellx);
	m_free(img_dilated);
	m_free(strel);
	m_free(max_gicov);
	m_free(gicov);
	m_free(grad_y);
	m_free(grad_x);

	// Report the total number of cells detected
	printf("Cells detected: %d\n\n", k_count);

	// Report the breakdown of the detection runtime
	printf("Detection runtime\n");
	printf("-----------------\n");
	printf("GICOV computation: %.5f seconds\n", ((float) (GICOV_end_time - GICOV_start_time)) / (1000*1000));
	printf("   GICOV dilation: %.5f seconds\n", ((float) (dilate_end_time - dilate_start_time)) / (1000*1000));
	printf("            Total: %.5f seconds\n", ((float) (get_time() - program_start_time)) / (1000*1000));

	// Now that the cells have been detected in the first frame,
	//  track the ellipses through subsequent frames
	if (num_frames > 1) printf("\nTracking cells across %d frames\n", num_frames);
	else                printf("\nTracking cells across 1 frame\n");
	long long tracking_start_time = get_time();
	int num_snaxels = 20;
	ellipsetrack(cell_file, QAX_CENTERS, QAY_CENTERS, k_count, radius, num_snaxels, num_frames);
	printf("           Total: %.5f seconds\n", ((float) (get_time() - tracking_start_time)) / (float) (1000*1000*num_frames));

	// Report total program execution time
    printf("\nTotal application run time: %.5f seconds\n", ((float) (get_time() - program_start_time)) / (1000*1000));

	free(QAX_CENTERS);
	free(QAY_CENTERS);
	AVI_close(cell_file);
	return 0;

cleanup_failure:
	free(V);
	free(QAX_CENTERS);
	free(QAY_CENTERS);
	free(t);
	free(G);
	free(x_result);
	free(y_result);
	free(crow);
	free(ccol);
	free(GICOV_spots);
	m_free(A);
	m_free(celly);
	m_free(cellx);
	m_free(img_dilated);
	m_free(strel);
	m_free(max_gicov);
	m_free(gicov);
	m_free(grad_y);
	m_free(grad_x);
	AVI_close(cell_file);
	return EXIT_FAILURE;
}
