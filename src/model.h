#ifndef MODEL_H_
#define MODEL_H_

void model_init(void);
void model_draw(void);
const float *model_matrix(void);
void model_pan_start(int x, int y);
void model_pan_move(int x, int y);


#endif /* MODEL_H_ */
