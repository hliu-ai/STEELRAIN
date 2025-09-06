#ifndef OBJECT_TYPE_HPP
#define OBJECT_TYPE_HPP

enum class ObjectType
{
    None = 0,     // No collision
    Banana = 1,   // Collidable with Banana
    Platform = 2  // Collidable with Platform
};

#endif // OBJECT_TYPE_HPP
