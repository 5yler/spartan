#pragma once


#include <vtkSmartPointer.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataPointSampler.h>
#include <vtkCleanPolyData.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>
#include <vtkSTLReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtksys/SystemTools.hxx>

// Generic loader that loads common model formats into
// a vtkPolyData object.
static vtkSmartPointer<vtkPolyData> ReadPolyData(const char *fileName)
{
  vtkSmartPointer<vtkPolyData> polyData;
  std::string extension = vtksys::SystemTools::GetFilenameExtension(std::string(fileName));
  if (extension == ".ply")
  {
    vtkSmartPointer<vtkPLYReader> reader =
      vtkSmartPointer<vtkPLYReader>::New();
    reader->SetFileName (fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".vtp")
  {
    vtkSmartPointer<vtkXMLPolyDataReader> reader =
      vtkSmartPointer<vtkXMLPolyDataReader>::New();
    reader->SetFileName (fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".obj")
  {
    vtkSmartPointer<vtkOBJReader> reader =
      vtkSmartPointer<vtkOBJReader>::New();
    reader->SetFileName (fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".stl")
  {
    vtkSmartPointer<vtkSTLReader> reader =
      vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName (fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  return polyData;
}

static Eigen::Matrix3Xd LoadAndDownsamplePolyData(const std::string fileName, double downsample_spacing = -1.0)
{
  vtkSmartPointer<vtkPolyData> cloudPolyData = ReadPolyData(fileName.c_str());
  cout << "Loaded " << cloudPolyData->GetNumberOfPoints() << " points from " << fileName << endl;

  vtkSmartPointer<vtkPolyData> cloudPolyDataOut;

  if (downsample_spacing <= 0.0){
    cloudPolyDataOut = cloudPolyData;
    printf("... and not downsampling.\n");
  } else {
    vtkSmartPointer<vtkPolyDataPointSampler> pointSampler = vtkSmartPointer<vtkPolyDataPointSampler>::New();
    pointSampler->SetDistance(downsample_spacing);
    pointSampler->SetInput(cloudPolyData);
    pointSampler->Update();
    vtkSmartPointer<vtkPolyData> cloudPolyDataDownsampled = pointSampler->GetOutput();

    printf("sampled but not cleaned\n");
    vtkSmartPointer<vtkCleanPolyData> pointCleaner = vtkSmartPointer<vtkCleanPolyData>::New();
    pointCleaner->SetToleranceIsAbsolute(true);
    pointCleaner->SetAbsoluteTolerance(downsample_spacing);
    pointCleaner->SetInput(cloudPolyDataDownsampled);
    pointCleaner->Update();
    cloudPolyDataOut = pointCleaner->GetOutput();

    cout << "Downsampled to " << cloudPolyDataOut->GetNumberOfPoints() << " points" << endl;
  }

  Eigen::Matrix3Xd out_pts(3, cloudPolyDataOut->GetNumberOfPoints());
  for (int i=0; i<cloudPolyDataOut->GetNumberOfPoints(); i++){
    out_pts(0, i) = cloudPolyDataOut->GetPoint(i)[0];
    out_pts(1, i) = cloudPolyDataOut->GetPoint(i)[1];
    out_pts(2, i) = cloudPolyDataOut->GetPoint(i)[2];
  }
  return out_pts;
}