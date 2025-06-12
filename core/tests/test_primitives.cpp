#include <gtest/gtest.h>
#include <spirographicals/spirographicals.h>

TEST(SpirocoreAPITest, InitializationAndTermination) {
    ASSERT_NO_THROW(sp_initialize());
    ASSERT_NO_THROW(sp_terminate());
}

TEST(SpirocoreAPITest, CanvasLifecycle) {
    sp_initialize();
    sp_window_config_t config = {100, 100, "Test", false, false};
    sp_canvas_t* canvas = sp_create_canvas(&config);
    ASSERT_NE(canvas, nullptr);
    ASSERT_FALSE(sp_canvas_should_close(canvas));
    sp_destroy_canvas(canvas);
    sp_terminate();
}

TEST(SpirocoreAPITest, NullCanvasIsHandledGracefully) {
    sp_canvas_t* canvas = nullptr;
    ASSERT_NO_THROW(sp_destroy_canvas(canvas));
    ASSERT_TRUE(sp_canvas_should_close(canvas));
    ASSERT_NO_THROW(sp_begin_frame(canvas));
    ASSERT_NO_THROW(sp_end_frame(canvas));
    ASSERT_NO_THROW(sp_draw_line(canvas, 0, 0, 1, 1));
}

TEST(SpirocoreAPITest, PenLifecycle) {
    sp_initialize();
    sp_window_config_t config = {100, 100, "Test", false, false};
    sp_canvas_t* canvas = sp_create_canvas(&config);
    
    sp_pen_config_t pen_config = {2.0f, SP_LINE_CAP_ROUND, SP_LINE_JOIN_ROUND, 10.0f};
    sp_pen_t* pen = sp_create_pen(canvas, &pen_config);
    ASSERT_NE(pen, nullptr);
    ASSERT_NO_THROW(sp_set_pen(canvas, pen));
    ASSERT_NO_THROW(sp_destroy_pen(pen));

    sp_destroy_canvas(canvas);
    sp_terminate();
}

TEST(SpirocoreAPITest, PathLifecycle) {
    sp_initialize();
    sp_window_config_t config = {100, 100, "Test", false, false};
    sp_canvas_t* canvas = sp_create_canvas(&config);

    sp_path_t* path = sp_create_path(canvas);
    ASSERT_NE(path, nullptr);
    ASSERT_NO_THROW(sp_path_move_to(path, 10, 10));
    ASSERT_NO_THROW(sp_path_line_to(path, 20, 20));
    ASSERT_NO_THROW(sp_stroke_path(canvas, path));
    ASSERT_NO_THROW(sp_destroy_path(path));

    sp_destroy_canvas(canvas);
    sp_terminate();
}
