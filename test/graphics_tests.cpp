#include <catch2/catch.hpp>

#include <wxpex/graphics.h>
#include <tau/random.h>
#include <tau/angles.h>


TEST_CASE("Identity matrix", "[graphics]")
{
    auto renderer = wxGraphicsRenderer::GetDefaultRenderer();

    // wxWidgets documentation claims that CreateMatrix returns an identity
    // matrix.
    auto matrix = renderer->CreateMatrix();

    // When I test this, it returns false.
    // REQUIRE(matrix.IsIdentity());

    auto graphicsMatrix = wxpex::GraphicsMatrix(matrix);
    auto scale = graphicsMatrix.GetScale();
    auto rotation = graphicsMatrix.GetRotation();
    auto translation = graphicsMatrix.GetTranslation();

    // When I wrap the matrix in my own test, IsIdentity is true.
    REQUIRE(graphicsMatrix.IsIdentity());
    REQUIRE(scale.vertical == 1.0);
    REQUIRE(scale.horizontal == 1.0);
    REQUIRE(rotation == 0.0);
    REQUIRE(translation.x == 0.0);
    REQUIRE(translation.y == 0.0);
}


TEST_CASE("SimpleRotation", "[graphics]")
{
    auto renderer = wxGraphicsRenderer::GetDefaultRenderer();
    auto matrix = renderer->CreateMatrix();

    auto quarterCircle = tau::Angles<double>::tau / 4.0;
    matrix.Rotate(quarterCircle);
    auto graphicsMatrix = wxpex::GraphicsMatrix(matrix);
    auto rotation = graphicsMatrix.GetRotation();

    REQUIRE(rotation == Approx(quarterCircle));
}


TEST_CASE("Rotation", "[graphics]")
{
    auto seed = GENERATE(
        take(16, random(tau::SeedLimits::min(), tau::SeedLimits::max())));

    tau::UniformRandom<double> uniformRandom{seed};
    uniformRandom.SetRange(-tau::Angles<double>::pi, tau::Angles<double>::pi);
    auto randomRotation = uniformRandom();

    auto renderer = wxGraphicsRenderer::GetDefaultRenderer();
    auto matrix = renderer->CreateMatrix();

    matrix.Rotate(randomRotation);
    auto graphicsMatrix = wxpex::GraphicsMatrix(matrix);
    auto rotation = graphicsMatrix.GetRotation();

    REQUIRE(rotation == Approx(randomRotation));
}


TEST_CASE("Scale", "[graphics]")
{
    auto seed = GENERATE(
        take(16, random(tau::SeedLimits::min(), tau::SeedLimits::max())));

    tau::UniformRandom<double> uniformRandom{seed};
    uniformRandom.SetRange(0.01, 10000.0);
    auto xScale = uniformRandom();
    auto yScale = uniformRandom();

    auto renderer = wxGraphicsRenderer::GetDefaultRenderer();
    auto matrix = renderer->CreateMatrix();

    matrix.Scale(xScale, yScale);
    auto graphicsMatrix = wxpex::GraphicsMatrix(matrix);
    auto scale = graphicsMatrix.GetScale();

    REQUIRE(scale.horizontal == Approx(xScale));
    REQUIRE(scale.vertical == Approx(yScale));
}


TEST_CASE("Extract translation/rotation/scale", "[graphics]")
{
    auto renderer = wxGraphicsRenderer::GetDefaultRenderer();
    auto matrix = renderer->CreateMatrix();

    auto seed = GENERATE(
        take(16, random(tau::SeedLimits::min(), tau::SeedLimits::max())));

    tau::UniformRandom<double> uniformRandom{seed};

    uniformRandom.SetRange(0.01, 1000.0);
    auto xScale = uniformRandom();
    auto yScale = uniformRandom();

    uniformRandom.SetRange(-10000.0, 10000.0);
    auto xTranslation = uniformRandom();
    auto yTranslation = uniformRandom();

    uniformRandom.SetRange(-tau::Angles<double>::pi, tau::Angles<double>::pi);
    auto randomRotation = uniformRandom();

    // Note: Order matters.
    matrix.Translate(xTranslation, yTranslation);
    matrix.Scale(xScale, yScale);
    matrix.Rotate(randomRotation);

    auto graphicsMatrix = wxpex::GraphicsMatrix(matrix);
    auto scale = graphicsMatrix.GetScale();
    auto rotation = graphicsMatrix.GetRotation();
    auto translation = graphicsMatrix.GetTranslation();

    REQUIRE(scale.vertical == Approx(yScale));
    REQUIRE(scale.horizontal == Approx(xScale));
    REQUIRE(rotation == Approx(randomRotation));
    REQUIRE(translation.x == Approx(xTranslation));
    REQUIRE(translation.y == Approx(yTranslation));
}
