#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "rex_engine/engine/types.h"
#include "rex_std/bonus/math/deg_angle.h"

// #TODO: Remaining cleanup of development/Pokemon -> main merge. ID: CAMERA

namespace rex
{
	namespace gfx
	{
		enum class ProjectionMode
		{
			Perspective,
			Ortographic
		};

		struct CameraDimensions
		{
			rsl::deg_angle fov;
			f32 width;
			f32 height;
			f32 near;
			f32 far;
		};

		// A basic camera class
		class Camera
		{
		public:
			// When the arguments for near and far plane used to be called "near" and "far"
			// A compiler error occured where those paremters would not be used to initialize the members
			// Leaving the members to be zero initialized
			// Renaming them to "nearPlane" and "farPlane" fixes the issue
			Camera(const glm::vec3& pos, const glm::vec3& fwd, const CameraDimensions& camDimensions, ProjectionMode projectionMode = ProjectionMode::Perspective);

			// Return the position of the camera
			const glm::vec3& position() const;
			// Return the forward vector of the camera
			const glm::vec3& forward() const;

			// Return the view matrix of the camera
			glm::mat4 view_mat() const;
			// Return the projection matrix of the camera
			const glm::mat4& projection_mat() const;

			// Switch the projection mode of the camera
			void switch_projection_mode(ProjectionMode newMode);

		private:
			// Calculate the projection matrix based on the current camera settings
			void calc_proj_matrix();

		private:
			glm::vec3 m_position;
			glm::vec3 m_forward;

			glm::mat4 m_projection;

			CameraDimensions m_cam_dimensions;
			ProjectionMode m_projection_mode;
		};
	}
}