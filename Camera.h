class Camera {
private:
	float pos[3];
	float target[3];
	float up[3];
	short type; // 0: perspective; 1: orthographic

public:
	Camera(float pos_x, float pos_y, float pos_z, float target_x, float target_y, float target_z, float up_x, float up_y, float up_z, short type) {
		pos[0] = pos_x, pos[1] = pos_y, pos[2] = pos_z;
		target[0] = target_x, target[1] = target_y, target[2] = target_z;
		up[0] = up_x, up[1] = up_y, up[2] = up_z;
		this->type = type;
	}

	float get_pos(int n) {
		return pos[n];
	}

	float get_target(int n) {
		return target[n];
	}

	float get_up(int n) {
		return up[n];
	}

	short get_type() {
		return type;
	}

	void update(float sleigh_pos[4], float sleigh_dir[4]) {
		pos[0] = sleigh_pos[0] - sleigh_dir[0] * 5.0f;
		pos[1] = sleigh_pos[1] - sleigh_dir[1] * 5.0f + 2.0f;
		pos[2] = sleigh_pos[2] - sleigh_dir[2] * 5.0f;
		target[0] = sleigh_pos[0], target[1] = sleigh_pos[1], target[2] = sleigh_pos[2];
	}
};