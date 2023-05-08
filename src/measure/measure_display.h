/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "measure_tool.h"
#include "../base/string_conv.h"
#include "../graphics/graphics_object_ptr.h"

#include <AIS_Circle.hxx>
#include <AIS_Line.hxx>
#include <AIS_Point.hxx>
#include <AIS_TextLabel.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <gp_Circ.hxx>
#include <Quantity_Color.hxx>

#include <memory>
#include <string>
#include <string_view>

namespace Mayo {

// Provides parameters to configure IMeasureDisplay objects
struct MeasureDisplayConfig {
    LengthUnit lengthUnit = LengthUnit::Millimeter;
    AngleUnit angleUnit = AngleUnit::Degree;
    AreaUnit areaUnit = AreaUnit::SquareMillimeter;
    DoubleToStringOptions doubleToStringOptions;
    double devicePixelRatio = 1.;
};

// Provides an interface to textual/graphics representation of a measure
class IMeasureDisplay {
public:
    virtual ~IMeasureDisplay() = default;

    // Update the textual and graphical representations regarding some display configuration
    virtual void update(const MeasureDisplayConfig& config) = 0;

    // Textual representation of the measure
    virtual std::string text() const = 0;

    // Count of objects for the 3D graphics representation of the measure
    virtual int graphicsObjectsCount() const = 0;

    // 3D graphics object at index 'i'
    // Valid index is within [0 .. graphicsObjectsCount[
    virtual GraphicsObjectPtr graphicsObjectAt(int i) const = 0;

    // Adapt 3D graphics objects to what is supported by 'driver'
    // This function must be called before adding the graphical objects to the 3D scene(Mayo::GraphicsScene
    // or AIS_InteractiveContext)
    virtual void adaptGraphics(const Handle_Graphic3d_GraphicDriver& driver) = 0;

    // Whether "sum" mode is supported by the measure display
    // This is relevant when multiple measureable 3D objects are selected. The cumulative sum of
    // each measure is computed and made available in the textual and/or graphics representations
    virtual bool isSumSupported() const = 0;

    // Add 'other' to this measure display(see isSumSupported())
    // 'other' should be of the same base type as this IMeasureDisplay object
    virtual void sumAdd(const IMeasureDisplay& other) = 0;
};

// Base class for IMeasureDisplay implementations
class BaseMeasureDisplay : public IMeasureDisplay {
public:
    std::string text() const override { return m_text; }

    // Factory method to create an IMeasureDisplay object suited to input measure value
    static std::unique_ptr<IMeasureDisplay> createFrom(MeasureType type, const MeasureValue& value);
    static std::unique_ptr<IMeasureDisplay> createEmptySumFrom(MeasureType type);

    void adaptGraphics(const Handle_Graphic3d_GraphicDriver& driver) override;

    bool isSumSupported() const override { return false; }
    void sumAdd(const IMeasureDisplay& other) override;

protected:
    void setText(std::string_view str) { m_text = str; }

    int sumCount() const { return m_sumCount; }
    std::string_view sumTextOr(std::string_view singleItemText) const;

    static std::string text(const gp_Pnt& pnt, const MeasureDisplayConfig& config);
    static std::string text(double value, const MeasureDisplayConfig& config);
    static std::string graphicsText(const gp_Pnt& pnt, const MeasureDisplayConfig& config);
    static void adaptScale(const Handle_AIS_TextLabel& gfxText, const MeasureDisplayConfig& config);

    static void applyGraphicsDefaults(IMeasureDisplay* measureDisplay);

private:
    std::string m_text;
    int m_sumCount = 0;
};

// --
// -- Vertex
// --

class MeasureDisplayVertex : public BaseMeasureDisplay {
public:
    MeasureDisplayVertex(const gp_Pnt& pnt);
    void update(const MeasureDisplayConfig& config) override;
    int graphicsObjectsCount() const override { return 1; }
    GraphicsObjectPtr graphicsObjectAt(int i) const override;

private:
    gp_Pnt m_pnt;
    Handle_AIS_TextLabel m_gfxText;
};

// --
// -- CircleCenter
// --

class MeasureDisplayCircleCenter : public BaseMeasureDisplay {
public:
    MeasureDisplayCircleCenter(const MeasureCircle& circle);
    void update(const MeasureDisplayConfig& config) override;
    int graphicsObjectsCount() const override;
    GraphicsObjectPtr graphicsObjectAt(int i) const override;

private:
    gp_Circ m_circle;
    Handle_AIS_Point m_gfxPoint;
    Handle_AIS_TextLabel m_gfxText;
    Handle_AIS_Circle m_gfxCircle;
};

// --
// -- CircleDiameter
// --

class MeasureDisplayCircleDiameter : public BaseMeasureDisplay {
public:
    MeasureDisplayCircleDiameter(const MeasureCircle& circle);
    void update(const MeasureDisplayConfig& config) override;
    int graphicsObjectsCount() const override { return 3; }
    GraphicsObjectPtr graphicsObjectAt(int i) const override;

private:
    static gp_Pnt diameterOpposedPnt(const gp_Pnt& pntOnCircle, const gp_Circ& circ);

    gp_Circ m_circle;
    Handle_AIS_Circle m_gfxCircle;
    Handle_AIS_Line m_gfxDiameter;
    Handle_AIS_TextLabel m_gfxDiameterText;
};

// --
// -- MinDistance
// --

class MeasureDisplayMinDistance : public BaseMeasureDisplay {
public:
    MeasureDisplayMinDistance(const MeasureMinDistance& dist);
    void update(const MeasureDisplayConfig& config) override;
    int graphicsObjectsCount() const override { return 4; }
    GraphicsObjectPtr graphicsObjectAt(int i) const override;

private:
    MeasureMinDistance m_dist;
    Handle_AIS_Line m_gfxLength;
    Handle_AIS_TextLabel m_gfxDistText;
    Handle_AIS_Point m_gfxPnt1;
    Handle_AIS_Point m_gfxPnt2;
};

// --
// -- Angle
// --

class MeasureDisplayAngle : public BaseMeasureDisplay {
public:
    MeasureDisplayAngle(MeasureAngle angle);
    void update(const MeasureDisplayConfig& config) override;
    int graphicsObjectsCount() const override;
    GraphicsObjectPtr graphicsObjectAt(int i) const override;

private:
    MeasureAngle m_angle;
    Handle_AIS_Line m_gfxEntity1;
    Handle_AIS_Line m_gfxEntity2;
    Handle_AIS_Circle m_gfxAngle;
    Handle_AIS_TextLabel m_gfxAngleText;
};

// --
// -- Length
// --

class MeasureDisplayLength : public BaseMeasureDisplay {
public:
    MeasureDisplayLength(const MeasureLength& length);
    void update(const MeasureDisplayConfig& config) override;
    int graphicsObjectsCount() const override { return 1; }
    GraphicsObjectPtr graphicsObjectAt(int i) const override;

    bool isSumSupported() const override { return true; }
    void sumAdd(const IMeasureDisplay& other) override;

private:
    MeasureLength m_length;
    Handle_AIS_TextLabel m_gfxLenText;
};

// --
// -- Area
// --

class MeasureDisplayArea : public BaseMeasureDisplay {
public:
    MeasureDisplayArea(const MeasureArea& area);
    void update(const MeasureDisplayConfig& config) override;
    int graphicsObjectsCount() const override { return 1; }
    GraphicsObjectPtr graphicsObjectAt(int i) const override;

    bool isSumSupported() const override { return true; }
    void sumAdd(const IMeasureDisplay& other) override;

private:
    MeasureArea m_area;
    Handle_AIS_TextLabel m_gfxAreaText;
};

} // namespace Mayo
