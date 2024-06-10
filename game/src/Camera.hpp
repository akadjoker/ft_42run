#pragma once



const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  58.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;


enum Movement 
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera
{

    public:
        Camera()
        {
            position = Vec3(0.0f, 1.0f, -5.0f);
            front = Vec3(0.0f, 0.0f, 1.0f);
            right = Vec3(1.0f, 0.0f, 0.0f);
            worldUp = Vec3(0.0f, 1.0f, 0.0f);
            up = Vec3(0.0f, 1.0f, 0.0f);
            Yaw = 90.0f;
            Pitch = 0.0f;
            Zoom = 45.0f;
            near = 0.1f;
            far = 2000.0f;
            movementSpeed = SPEED;
            mouseSensitivity = SENSITIVITY;
            
        }
        Camera(Vec3 position, Vec3 target)
        {
            this->position = position;
            front = Vec3::Normalize(target - position);
            right = Vec3(1.0f, 0.0f, 0.0f);
            worldUp = Vec3(0.0f, 1.0f, 0.0f);
            up = Vec3(0.0f, 1.0f, 0.0f);
            Yaw = 0.0f;
            Pitch = 0.0f;
            Zoom = 45.0f;
            near = 0.1f;
            far = 2000.0f;
            movementSpeed = SPEED;
            mouseSensitivity = SENSITIVITY;
    
        }
        ~Camera(){};

        void Init(Vec3 position, Vec3 target)
        {
            this->position = position;
            front = Vec3::Normalize(target - position);
            right = Vec3(1.0f, 0.0f, 0.0f);
            worldUp = Vec3(0.0f, 1.0f, 0.0f);
            up = Vec3(0.0f, 1.0f, 0.0f);
            Yaw = 0.0f;
            Pitch = 0.0f;
            Zoom = 45.0f;
            near = 0.1f;
            far = 2000.0f;
            movementSpeed = SPEED;
            mouseSensitivity = SENSITIVITY;
        }
       
        Mat4 GetViewMatrix()
        {
                  
            update();
            return  Mat4::LookAt(position, position +  front, up);
            
        }

        Mat4 GetProjectionMatrix(float aspect)
        {
            return Mat4::Perspective(Zoom, aspect, near, far);
        }

    void ProcessMouseScroll(float yoffset)
    {
       Zoom -= (float)yoffset;
        if (Zoom < 45.0f)
            Zoom = 45.0f;
        if (Zoom > 95.0f)
            Zoom = 95.0f;
    }

     void ProcessKeyboard(Movement direction, float deltaTime)
    {
        float velocity = movementSpeed * deltaTime;
        if (direction == FORWARD)
            position += front * velocity;
        if (direction == BACKWARD)
            position -= front * velocity;
        if (direction == LEFT)
            position -= right * velocity;
        if (direction == RIGHT)
            position += right * velocity;
       
    
    }


    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        

    }

    float getNearClip() const
    {
        return near;
    }

    float getFarClip() const
    {
        return far;
    }


    Vec3 position;
    Vec3 front;
    Vec3 up;
    Vec3 right; 
        float Yaw;
    float Pitch;
    private:
   
    Vec3 worldUp;

    float movementSpeed;
    float mouseSensitivity;

    float Zoom;
    float near;
    float far;

    Mat4 view;

    void update()
    {
        Vec3 direction;
        direction.x = cos(ToRadians(Yaw)) * cos(ToRadians(Pitch));
        direction.y = sin(ToRadians(Pitch));
        direction.z = sin(ToRadians(Yaw)) * cos(ToRadians(Pitch));
        front = Vec3::Normalize(direction);

        right =Vec3::Normalize(Vec3::Cross(front, worldUp));
        up    =Vec3::Normalize(Vec3::Cross(right, front));
    
    }


};


class FollowCamera
{

    public:
        FollowCamera()
        {
            targetPosition = Vec3(0.0f, 0.0f, 0.0f);
            smoothness = 0.0f;
            cameraOffset = Vec3(0.0f, 0.0f, 0.0f);
        }

        FollowCamera(Vec3 cameraOffset,float smoothness)
        {
            this->cameraOffset = cameraOffset;
            this->targetPosition = targetPosition;
            this->smoothness = smoothness;
        }

        void Init(Vec3 cameraOffset,float smoothness)
        {
            this->cameraOffset = cameraOffset;
            this->targetPosition = targetPosition;
            this->smoothness = smoothness;
        }


        void Update(float deltaTime, Vec3 targetPosition)
        {
            this->targetPosition = targetPosition;
          
            
            Vec3 targetOffset = targetPosition + cameraOffset;
            currentPosition =  Vec3::Lerp(currentPosition, targetOffset, smoothness * deltaTime);


        }

        Mat4 GetViewMatrix()
        {
            return  Mat4::LookAt(currentPosition,  targetPosition, Vec3(0.0f, 1.0f, 0.0f));
        }


    private:
        Vec3 targetPosition;
        Vec3 cameraOffset;
        Vec3 currentPosition;
        float smoothness;

};


class FixCamera 
{
public:
    FixCamera()
    {
        m_TargetPosition = Vec3(0.0f, 0.0f, 0.0f);
        m_CameraOffset = Vec3(0.0f, 0.0f, 0.0f);
    }
    FixCamera(Vec3 cameraOffset) : m_CameraOffset(cameraOffset) {}

    void Init(Vec3 cameraOffset) 
    {
        m_CameraOffset = cameraOffset;
    }

    void Update(Vec3 targetPosition) 
    {
        m_TargetPosition = targetPosition;
    }

    Mat4 GetViewMatrix() const 
    {
         return  Mat4::LookAt(m_TargetPosition + m_CameraOffset, m_TargetPosition, Vec3(0.0f, 1.0f, 0.0f));
    }

private:
    Vec3 m_TargetPosition; // Posição do alvo (objeto a ser seguido)
    Vec3 m_CameraOffset;   // Deslocamento da câmera em relação ao alvo
};