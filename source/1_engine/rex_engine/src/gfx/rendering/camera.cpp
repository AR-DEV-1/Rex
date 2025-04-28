#include "rex_engine/gfx/rendering/camera.h"

// #TODO: Remaining cleanup of development/Pokemon -> main merge. ID: CAMERA

namespace rex
{
	namespace gfx
	{
		Camera::Camera(const glm::vec3& pos, const glm::vec3& fwd, const CameraDimensions& camDimensions, ProjectionMode projectionMode)
			: m_position(pos)
			, m_forward(fwd)
			, m_projection(1.0f)
			, m_projection_mode(projectionMode)
			, m_cam_dimensions(camDimensions)
		{
			calc_proj_matrix();
		}

		const glm::vec3& Camera::position() const
		{
			return m_position;
		}
		const glm::vec3& Camera::forward() const
		{
			return m_forward;
		}

		glm::mat4 Camera::view_mat() const
		{
			const glm::vec3 target = m_position + m_forward;
			const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

			return glm::lookAt(m_position, target, up);
		}
		const glm::mat4& Camera::projection_mat() const
		{
			return m_projection;
		}

		void Camera::switch_projection_mode(ProjectionMode newMode)
		{
			if (newMode != m_projection_mode)
			{
				m_projection_mode = newMode;
				calc_proj_matrix();
			}
		}

		void Camera::calc_proj_matrix()
		{
			f32 fov_rad = glm::radians(m_cam_dimensions.fov.get());

			switch (m_projection_mode)
			{
			case ProjectionMode::Perspective:
				m_projection = glm::perspectiveFov(fov_rad, m_cam_dimensions.width, m_cam_dimensions.height, m_cam_dimensions.near_plane, m_cam_dimensions.far_plane);
				break;
			case ProjectionMode::Ortographic:
			{
				// Calculate the dimension of the near plane
				f32 h = glm::cos(0.5f * fov_rad) / glm::sin(0.5f * fov_rad);
				f32 w = 2 * h * m_cam_dimensions.height / m_cam_dimensions.width;
				f32 left = -w;
				f32 right = w;
				f32 top = -h;
				f32 bottom = h;

				m_projection = glm::ortho(left, right, top, bottom, m_cam_dimensions.near_plane, m_cam_dimensions.far_plane);
				break;
			}
			}
		}
	}
}