// Test cases for the image class and related features.
// Written by Simon Fuhrmann.

#include <gtest/gtest.h>

#include "mve/image.h"
#include "mve/view.h"

TEST(ViewTest, AddSetHasRemoveTest)
{
    mve::View view;

    mve::ByteImage::Ptr image = mve::ByteImage::create(100, 100, 1);
    EXPECT_FALSE(view.has_image("image"));
    EXPECT_EQ(0, view.get_images().size());
    EXPECT_NO_THROW(view.set_image(image, "image"));
    EXPECT_EQ(1, view.get_images().size());
    EXPECT_NO_THROW(view.set_image(image, "image"));
    EXPECT_EQ(1, view.get_images().size());
    EXPECT_TRUE(view.has_image("image"));
    EXPECT_TRUE(view.remove_image("image"));
    EXPECT_FALSE(view.remove_image("image"));
    EXPECT_FALSE(view.has_image("image"));
    EXPECT_EQ(0, view.get_images().size());

    mve::ByteImage::Ptr blob = mve::ByteImage::create(100, 1, 1);
    EXPECT_FALSE(view.has_blob("blob"));
    EXPECT_EQ(0, view.get_blobs().size());
    EXPECT_NO_THROW(view.set_blob(blob, "blob"));
    EXPECT_EQ(1, view.get_blobs().size());
    EXPECT_NO_THROW(view.set_blob(blob, "blob"));
    EXPECT_EQ(1, view.get_blobs().size());
    EXPECT_TRUE(view.has_blob("blob"));
    EXPECT_TRUE(view.remove_blob("blob"));
    EXPECT_FALSE(view.remove_blob("blob"));
    EXPECT_FALSE(view.has_blob("blob"));
    EXPECT_EQ(0, view.get_blobs().size());
}

TEST(ViewTest, AddRemoveMemorySizeTest)
{
    mve::ByteImage::Ptr image = mve::ByteImage::create(100, 100, 1);
    mve::ByteImage::Ptr blob = mve::ByteImage::create(100, 1, 1);
    mve::View view;
    view.set_image(image, "image");
    EXPECT_EQ(100 * 100, view.get_byte_size());
    view.set_blob(blob, "blob");
    EXPECT_EQ(100 * 100 + 100, view.get_byte_size());
    view.remove_image("image");
    EXPECT_EQ(100, view.get_byte_size());
    view.remove_blob("blob");
    EXPECT_EQ(0, view.get_byte_size());
}

TEST(ViewTest, IsDirtyTest)
{
    mve::ByteImage::Ptr image = mve::ByteImage::create(100, 100, 1);
    mve::ByteImage::Ptr blob = mve::ByteImage::create(100, 1, 1);
    mve::View view1, view2, view3;

    EXPECT_FALSE(view1.is_dirty());
    view1.set_image(image, "image");
    EXPECT_TRUE(view1.is_dirty());

    EXPECT_FALSE(view2.is_dirty());
    view2.set_blob(blob, "blob");
    EXPECT_TRUE(view2.is_dirty());

    EXPECT_FALSE(view3.is_dirty());
    EXPECT_FALSE(view3.get_meta_data().is_dirty);
    view3.set_value("view.key", "value");
    EXPECT_TRUE(view3.is_dirty());
    EXPECT_TRUE(view3.get_meta_data().is_dirty);
}

TEST(ViewTest, CacheCleanupTest)
{
    mve::ByteImage::Ptr image = mve::ByteImage::create(100, 1, 1);
    mve::ByteImage::Ptr blob = mve::ByteImage::create(100, 1, 1);
    mve::View view;
    view.set_image(image, "image");
    view.set_blob(blob, "blob");

    mve::View::ImageProxy const* image_proxy = view.get_image_proxy("image");
    mve::View::BlobProxy const* blob_proxy = view.get_blob_proxy("blob");

    EXPECT_TRUE(image_proxy->image != NULL);
    EXPECT_TRUE(blob_proxy->blob != NULL);
    EXPECT_EQ(0, view.cache_cleanup());
    EXPECT_TRUE(image_proxy->image != NULL);
    EXPECT_TRUE(blob_proxy->blob != NULL);
    image.reset();
    blob.reset();
    /* It is not cleaning anything because image and blob are dirty. */
    EXPECT_EQ(0, view.cache_cleanup());
    EXPECT_TRUE(image_proxy->image != NULL);
    EXPECT_TRUE(blob_proxy->blob != NULL);
}

TEST(ViewTest, KeyValueTest)
{
    mve::View view;
    EXPECT_THROW(view.set_value("", ""), std::exception);
    EXPECT_THROW(view.set_value("key", ""), std::exception);
    EXPECT_NO_THROW(view.set_value("section.key", ""));
    EXPECT_THROW(view.set_value("", "value"), std::exception);
    EXPECT_NO_THROW(view.set_value("section.key", "value"));

    EXPECT_THROW(view.get_value(""), std::exception);
    EXPECT_THROW(view.get_value("key"), std::exception);
    EXPECT_NO_THROW(view.get_value("section.key"));

    EXPECT_EQ(view.get_value("section.key2"), "");
    EXPECT_EQ(view.get_value("section.key"), "value");
    view.delete_value("section.key");
    EXPECT_EQ(view.get_value("section.key"), "");
}

TEST(ViewTest, GetByTypeTest)
{
    mve::FloatImage::Ptr image = mve::FloatImage::create(10, 12, 1);
    mve::View view;

    EXPECT_TRUE(view.get_image_proxy("image") == NULL);
    EXPECT_TRUE(view.get_image_proxy("image", mve::IMAGE_TYPE_FLOAT) == NULL);
    EXPECT_FALSE(view.has_image("image"));
    EXPECT_FALSE(view.has_image("image", mve::IMAGE_TYPE_UNKNOWN));
    EXPECT_FALSE(view.has_image("image", mve::IMAGE_TYPE_FLOAT));

    view.set_image(image, "image");

    EXPECT_TRUE(view.get_image("image", mve::IMAGE_TYPE_UINT8) == NULL);
    EXPECT_EQ(image, view.get_image("image", mve::IMAGE_TYPE_FLOAT));
    EXPECT_EQ(image, view.get_image("image", mve::IMAGE_TYPE_UNKNOWN));
    EXPECT_EQ(image, view.get_image("image"));

    EXPECT_TRUE(view.get_image_proxy("image", mve::IMAGE_TYPE_FLOAT) != NULL);
    EXPECT_TRUE(view.get_image_proxy("image", mve::IMAGE_TYPE_UINT8) == NULL);
    EXPECT_TRUE(view.get_image_proxy("image", mve::IMAGE_TYPE_UNKNOWN) != NULL);
    EXPECT_TRUE(view.get_image_proxy("image") != NULL);

    EXPECT_TRUE(view.has_image("image", mve::IMAGE_TYPE_UNKNOWN));
    EXPECT_TRUE(view.has_image("image", mve::IMAGE_TYPE_FLOAT));
    EXPECT_FALSE(view.has_image("image", mve::IMAGE_TYPE_UINT8));
    EXPECT_TRUE(view.has_image("image"));
}

TEST(ViewTest, GetTypeImageTest)
{
    mve::View view;

    EXPECT_EQ(mve::FloatImage::Ptr(), view.get_float_image("image"));
    EXPECT_EQ(mve::ByteImage::Ptr(), view.get_byte_image("image"));

    mve::FloatImage::Ptr image = mve::FloatImage::create(10, 12, 1);
    view.set_image(image, "image");
    EXPECT_EQ(image, view.get_float_image("image"));
    EXPECT_EQ(mve::ByteImage::Ptr(), view.get_byte_image("image"));

    mve::ByteImage::Ptr image2 = mve::ByteImage::create(10, 12, 1);
    view.set_image(image2, "image2");
    EXPECT_EQ(image2, view.get_byte_image("image2"));
    EXPECT_EQ(mve::FloatImage::Ptr(), view.get_float_image("image2"));
}

TEST(ViewTest, GetSetNameIdCamera)
{
    mve::View view;
    EXPECT_EQ(view.get_name(), "");
    view.set_name("testname");
    EXPECT_EQ(view.get_name(), "testname");

    EXPECT_EQ(view.get_id(), -1);
    view.set_id(12);
    EXPECT_EQ(view.get_id(), 12);

    EXPECT_FALSE(view.is_camera_valid());
    mve::CameraInfo camera;
    camera.flen = 1.0f;
    view.set_camera(camera);
    EXPECT_TRUE(view.is_camera_valid());
    EXPECT_EQ(view.get_camera().flen, camera.flen);
}
