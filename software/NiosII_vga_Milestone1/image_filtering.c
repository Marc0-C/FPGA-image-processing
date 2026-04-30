void flip_image(char image_data[], char image_return[], int axis, int size){

	int im_width = 320;
	int im_height = 240;
	int im_size = im_width*im_height;

	im_width >>= size;
	im_height >>= size;


	for (int i = 0; i < im_size; i += im_width){
		for (int j = 0; j < im_width; j += 1){
			if (axis == 0) {
				image_return[j + i] = image_data[i+im_width-1-j];
			} else {
				image_return[j + i] = image_data[j+im_size-im_width-i];
			}
		}
	}
	return;
}

