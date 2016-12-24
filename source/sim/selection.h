

class Matrix;

extern unsigned int g_circle;

void DrawSelectionCircles(Matrix* projection, Matrix* modelmat, Matrix* viewmat);
void DoSelection(Vec3f campos, Vec3f camside, Vec3f camup2, Vec3f viewdir);