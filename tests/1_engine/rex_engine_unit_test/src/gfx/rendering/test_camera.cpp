#include "rex_unit_test/rex_catch2.h"

#include "rex_engine/gfx/rendering/camera.h"

TEST_CASE("TEST - Camera - Origin Perspective Construction")
{
	glm::vec3 pos{};
	glm::vec3 forward(0.0f, 0.0f, 1.0f);
	rex::gfx::CameraDimensions dimensions{};
	dimensions.fov = 90_deg;
	dimensions.width = 720.0f;
	dimensions.height = 720.0f;
	dimensions.near_plane = 1.0f;
	dimensions.far_plane = 100.0f;

	rex::gfx::Camera camera(pos, forward, dimensions, rex::gfx::ProjectionMode::Perspective);

	glm::vec3 target = pos + forward;
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0);

	glm::mat4 view_mat = glm::lookAt(pos, target, up);

	REX_CHECK(camera.position() == pos);
	REX_CHECK(camera.forward() == forward);
	REX_CHECK(camera.view_mat() == view_mat);
	REX_CHECK(camera.projection_mat() == glm::perspectiveFov(glm::radians(dimensions.fov.get()), dimensions.width, dimensions.height, dimensions.near_plane, dimensions.far_plane));
}

TEST_CASE("TEST - Camera - Origin Orthographic Construction")
{
	glm::vec3 pos{};
	glm::vec3 forward(0.0f, 0.0f, 1.0f);
	rex::gfx::CameraDimensions dimensions{};
	dimensions.fov = 90_deg;
	dimensions.width = 720.0f;
	dimensions.height = 720.0f;
	dimensions.near_plane = 1.0f;
	dimensions.far_plane = 100.0f;

	rex::gfx::Camera camera(pos, forward, dimensions, rex::gfx::ProjectionMode::Ortographic);

	glm::vec3 target = pos + forward;
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0);

	glm::mat4 view_mat = glm::lookAt(pos, target, up);

	// Calculate the dimension of the near_plane plane
	f32 fov_rad = glm::radians(dimensions.fov.get());
	f32 h = glm::cos(0.5f * fov_rad) / glm::sin(0.5f * fov_rad);
	f32 w = 2 * h * dimensions.height / dimensions.width;
	f32 left = -w;
	f32 right = w;
	f32 top = -h;
	f32 bottom = h;

	REX_CHECK(camera.position() == pos);
	REX_CHECK(camera.forward() == forward);
	REX_CHECK(camera.view_mat() == view_mat);
	REX_CHECK(camera.projection_mat() == glm::ortho(left, right, top, bottom, dimensions.near_plane, dimensions.far_plane));
}

TEST_CASE("TEST - Camera - Perspective Construction")
{
	glm::vec3 pos(1.0f, 0.0f, 0.0f);
	glm::vec3 forward(0.0f, 0.0f, 1.0f);
	rex::gfx::CameraDimensions dimensions{};
	dimensions.fov = 90_deg;
	dimensions.width = 720.0f;
	dimensions.height = 720.0f;
	dimensions.near_plane = 1.0f;
	dimensions.far_plane = 100.0f;

	rex::gfx::Camera camera(pos, forward, dimensions, rex::gfx::ProjectionMode::Perspective);

	glm::vec3 target = pos + camera.forward();
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0);

	glm::mat4 view_mat = glm::lookAt(pos, target, up);

	REX_CHECK(camera.position() == pos);
	REX_CHECK(camera.forward() == -pos);
	REX_CHECK(camera.view_mat() == view_mat);
	REX_CHECK(camera.projection_mat() == glm::perspectiveFov(glm::radians(dimensions.fov.get()), dimensions.width, dimensions.height, dimensions.near_plane, dimensions.far_plane));
}

TEST_CASE("TEST - Camera - Orthographic Construction")
{
	glm::vec3 pos(1.0f, 0.0f, 0.0f);
	glm::vec3 forward(0.0f, 0.0f, 1.0f);
	rex::gfx::CameraDimensions dimensions{};
	dimensions.fov = 90_deg;
	dimensions.width = 720.0f;
	dimensions.height = 720.0f;
	dimensions.near_plane = 1.0f;
	dimensions.far_plane = 100.0f;

	rex::gfx::Camera camera(pos, forward, dimensions, rex::gfx::ProjectionMode::Ortographic);

	glm::vec3 target = pos + camera.forward();
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0);

	glm::mat4 view_mat = glm::lookAt(pos, target, up);

	// Calculate the dimension of the near_plane plane
	f32 fov_rad = glm::radians(dimensions.fov.get());
	f32 h = glm::cos(0.5f * fov_rad) / glm::sin(0.5f * fov_rad);
	f32 w = 2 * h * dimensions.height / dimensions.width;
	f32 left = -w;
	f32 right = w;
	f32 top = -h;
	f32 bottom = h;


	REX_CHECK(camera.position() == pos);
	REX_CHECK(camera.forward() == -pos);
	REX_CHECK(camera.view_mat() == view_mat);
	REX_CHECK(camera.projection_mat() == glm::ortho(left, right, top, bottom, dimensions.near_plane, dimensions.far_plane));
}

TEST_CASE("TEST - Camera - Distant Perspective Construction")
{
	glm::vec3 pos(10.0f, 0.0f, 0.0f);
	glm::vec3 forward(0.0f, 0.0f, 1.0f);
	rex::gfx::CameraDimensions dimensions{};
	dimensions.fov = 90_deg;
	dimensions.width = 720.0f;
	dimensions.height = 720.0f;
	dimensions.near_plane = 1.0f;
	dimensions.far_plane = 100.0f;

	rex::gfx::Camera camera(pos, forward, dimensions, rex::gfx::ProjectionMode::Perspective);

	glm::vec3 target = pos + camera.forward();
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0);

	glm::mat4 view_mat = glm::lookAt(pos, target, up);

	REX_CHECK(camera.position() == pos);
	REX_CHECK(camera.forward() == -pos);
	REX_CHECK(camera.view_mat() == view_mat);
	REX_CHECK(camera.projection_mat() == glm::perspectiveFov(glm::radians(dimensions.fov.get()), dimensions.width, dimensions.height, dimensions.near_plane, dimensions.far_plane));
}

TEST_CASE("TEST - Camera - Distant Orthographic Construction")
{
	glm::vec3 pos(10.0f, 0.0f, 0.0f);
	glm::vec3 forward(0.0f, 0.0f, 1.0f);
	rex::gfx::CameraDimensions dimensions{};
	dimensions.fov = 90_deg;
	dimensions.width = 720.0f;
	dimensions.height = 720.0f;
	dimensions.near_plane = 1.0f;
	dimensions.far_plane = 100.0f;

	rex::gfx::Camera camera(pos, forward, dimensions, rex::gfx::ProjectionMode::Ortographic);

	glm::vec3 target = pos + camera.forward();
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0);

	glm::mat4 view_mat = glm::lookAt(pos, target, up);

	// Calculate the dimension of the near_plane plane
	f32 fov_rad = glm::radians(dimensions.fov.get());
	f32 h = glm::cos(0.5f * fov_rad) / glm::sin(0.5f * fov_rad);
	f32 w = 2 * h * dimensions.height / dimensions.width;
	f32 left = -w;
	f32 right = w;
	f32 top = -h;
	f32 bottom = h;


	REX_CHECK(camera.position() == pos);
	REX_CHECK(camera.forward() == -pos);
	REX_CHECK(camera.view_mat() == view_mat);
	REX_CHECK(camera.projection_mat() == glm::ortho(left, right, top, bottom, dimensions.near_plane, dimensions.far_plane));
}

TEST_CASE("TEST - Camera - Switching Between Project Modes")
{
	glm::vec3 pos{};
	glm::vec3 forward(0.0f, 0.0f, 1.0f);
	rex::gfx::CameraDimensions dimensions{};
	dimensions.fov = 90_deg;
	dimensions.width = 720.0f;
	dimensions.height = 720.0f;
	dimensions.near_plane = 1.0f;
	dimensions.far_plane = 100.0f;

	rex::gfx::Camera camera(pos, forward, dimensions, rex::gfx::ProjectionMode::Perspective);

	glm::vec3 target = pos + camera.forward();
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0);

	glm::mat4 view_mat = glm::lookAt(pos, target, up);

	REX_CHECK(camera.position() == pos);
	REX_CHECK(camera.forward() == forward);
	REX_CHECK(camera.view_mat() == view_mat);
	REX_CHECK(camera.projection_mat() == glm::perspectiveFov(glm::radians(dimensions.fov.get()), dimensions.width, dimensions.height, dimensions.near_plane, dimensions.far_plane));

	camera.switch_projection_mode(rex::gfx::ProjectionMode::Ortographic);

	// Calculate the dimension of the near_plane plane
	f32 fov_rad = glm::radians(dimensions.fov.get());
	f32 h = glm::cos(0.5f * fov_rad) / glm::sin(0.5f * fov_rad);
	f32 w = 2 * h * dimensions.height / dimensions.width;
	f32 left = -w;
	f32 right = w;
	f32 top = -h;
	f32 bottom = h;

	REX_CHECK(camera.position() == pos);
	REX_CHECK(camera.forward() == forward);
	REX_CHECK(camera.view_mat() == view_mat);
	REX_CHECK(camera.projection_mat() == glm::ortho(left, right, top, bottom, dimensions.near_plane, dimensions.far_plane));

	camera.switch_projection_mode(rex::gfx::ProjectionMode::Perspective);

	target = pos + forward;
	up = glm::vec3(0.0f, 1.0f, 0.0);

	view_mat = glm::lookAt(pos, target, up);

	REX_CHECK(camera.position() == pos);
	REX_CHECK(camera.forward() == forward);
	REX_CHECK(camera.view_mat() == view_mat);
	REX_CHECK(camera.projection_mat() == glm::perspectiveFov(glm::radians(dimensions.fov.get()), dimensions.width, dimensions.height, dimensions.near_plane, dimensions.far_plane));
}