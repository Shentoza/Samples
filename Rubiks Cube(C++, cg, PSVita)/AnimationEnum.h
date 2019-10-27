#ifndef AnimationEnum_H
#define AnimationEnum_H
/*	Data structure for basic geometry */
enum anim_enum
{
	ANIM_STATE_COUNTERCLOCKWISE = -1,
	ANIM_STATE_NO_ROTATION = 0,
	ANIM_STATE_CLOCKWISE = 1
};

enum animator_states
{
	ANIMATOR_STATE_ACTIVE,
	ANIMATOR_STATE_INACTIVE,
	ANIMATOR_STATE_DRAGGING,
	ANIMATOR_STATE_REVERTING
};
#endif