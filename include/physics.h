#ifndef TARI_PHYSICS
#define TARI_PHYSICS

typedef struct {
  double x;
  double y;
  double z;
} Vector3D;

typedef Vector3D Position;
typedef Vector3D Velocity;
typedef Vector3D Acceleration;
typedef Vector3D Gravity;

typedef struct {
  Position mPosition;
  Velocity mVelocity;
  Acceleration mAcceleration;
} PhysicsObject;

void handlePhysics(PhysicsObject* tObject);
void setMaxVelocity(Velocity tVelocity);
void setGravity(Gravity tGravity);
void resetPhysicsObject(PhysicsObject* tObject);
void resetPhysics();
void initPhysics();

Position makePosition(double x, double y, double z);

int isEmptyVelocity(Velocity tVelocity);
Velocity normalizeVelocity(Velocity tVelocity);

#endif
