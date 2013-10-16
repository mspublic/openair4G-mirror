double interp(double x, double *xs, double *ys, int count)
{
    int i;
    double dx, dy;

    if (x < xs[0]) {
        /* x is less than the minimum element
         * handle error here if you want */
        return ys[0]; /* return minimum element */
    }

    if (x > xs[count-1]) {
        return ys[count-1]; /* return maximum */
    }

    /* find i, such that xs[i] <= x < xs[i+1] */
    for (i = 0; i < count-1; i++) {
        if (xs[i+1] > x) {
            break;
        }
    }

    /* interpolate */
    dx = xs[i+1] - xs[i];
    dy = ys[i+1] - ys[i];
    return ys[i] + (x - xs[i]) * dy / dx;
}
